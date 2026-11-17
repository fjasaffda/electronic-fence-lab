/*-----------------------------------------------------------------------------
* @文件名        : env_sensor
* @描述          : 大气压、温湿度、光照强度、二氧化碳检测驱动（I2C2 总线）
* @作者          : 物联网项目组
* @日期          : 2024/05/01
* @版本          : V1.0.0
******************************************************************************/

#include "env_sensor.h"
#include "i2c_api.h"
#include "i2c.h"
#include "string.h"
#include "stdio.h"

/********************************内部定义**************************************/
#define ENV_I2C_PORT        i2c_2              /* 四个传感器共用 I2C2（PB13/PB14） */

#define CPS121_ADDR_W       (0x6C << 1)        /* 0xD8 */
#define CPS121_ADDR_R       ((0x6C << 1) | 1)  /* 0xD9 */

#define SHT30_ADDR_W        (0x44 << 1)        /* 0x88，ADDR 接地；若 ADDR 接高电平改成 0x45 */
#define SHT30_ADDR_R        ((0x44 << 1) | 1)  /* 0x89 */

#define BH1750_ADDR_W       (0x23 << 1)        /* 0x46 */
#define BH1750_ADDR_R       ((0x23 << 1) | 1)  /* 0x47 */

#define SGP30_ADDR_W        (0x58 << 1)        /* 0xB0 */
#define SGP30_ADDR_R        ((0x58 << 1) | 1)  /* 0xB1 */

ENV_DATA g_env_data;

/* I2C 读写封装：失败返回 0，成功返回 1 */
static uint8_t Env_I2C_Write(uint8_t dev_addr, uint8_t *buf, uint16_t len)
{
    if((dev_addr == 0) || (len == 0))
    {
        return 0;
    }
    i2c_write(ENV_I2C_PORT, dev_addr, buf, len);
    return 1;
}

static uint8_t *Env_I2C_Read(uint8_t dev_addr, uint16_t len)
{
    return i2c_read(ENV_I2C_PORT, dev_addr, len);
}

/********************************************************************************
* 名    称：SHT30_Crc8
* 功    能：SHT30 数据校验 CRC8（多项式 0x31，初值 0xFF）
* 参数说明：data：数据指针   len：数据长度
* 返 回 值：校验值
* 调用方法：内部函数
********************************************************************************/
static uint8_t SHT30_Crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    uint8_t i, bit;

    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(bit = 0; bit < 8; bit++)
        {
            if(crc & 0x80)
            {
                crc = (uint8_t)((crc << 1) ^ 0x31);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

/********************************************************************************
* 名    称：CPS121_Read
* 功    能：读取大气压值
* 参数说明：pressure：气压值，单位 hPa
* 返 回 值：1：读取成功  0：读取失败
* 说    明：先写 0x30 0x0A 启动单次测量，20ms 后读 0x06 寄存器的 3 个字节，
            24 位原始值 / 64 得到 Pa，再 / 100 换算成 hPa
* 调用方法：内部函数
********************************************************************************/
static uint8_t CPS121_Read(float *pressure)
{
    uint8_t cmd[2];
    uint8_t rx[3] = {0};
    uint32_t press = 0;

    cmd[0] = 0x30;
    cmd[1] = 0x0A;
    if(!Env_I2C_Write(CPS121_ADDR_W, cmd, 2))
    {
        return 0;
    }
    HAL_Delay(20);

    /* 读 0x06 寄存器：内部为“写地址 + 重复起始 + 读数据”，与 CPS121 时序一致 */
    if(HAL_I2C_Mem_Read(&hi2c2, CPS121_ADDR_W, 0x06, I2C_MEMADD_SIZE_8BIT, rx, 3, 100) != HAL_OK)
    {
        return 0;
    }

    press = ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | (uint32_t)rx[2];
    *pressure = ((float)press / 64.0f) / 100.0f;
    return 1;
}

/********************************************************************************
* 名    称：SHT30_Read
* 功    能：读取温度、湿度值
* 参数说明：temp：温度 ℃   hum：湿度 %RH
* 返 回 值：1：读取成功  0：读取失败
* 说    明：单次测量命令 0x24 0x00（高重复性、无时钟拉伸），20ms 后读 6 字节
* 调用方法：内部函数
********************************************************************************/
static uint8_t SHT30_Read(float *temp, float *hum)
{
    uint8_t cmd[2];
    uint8_t *rx = NULL;
    uint16_t raw_temp = 0;
    uint16_t raw_hum = 0;

    cmd[0] = 0x24;
    cmd[1] = 0x00;
    if(!Env_I2C_Write(SHT30_ADDR_W, cmd, 2))
    {
        return 0;
    }
    HAL_Delay(20);

    rx = Env_I2C_Read(SHT30_ADDR_R, 6);
    if(rx == NULL)
    {
        return 0;
    }

    /* 两个字节数据 + 一个字节 CRC 校验 */
    if((SHT30_Crc8(&rx[0], 2) != rx[2]) || (SHT30_Crc8(&rx[3], 2) != rx[5]))
    {
        return 0;
    }

    raw_temp = ((uint16_t)rx[0] << 8) | (uint16_t)rx[1];
    raw_hum  = ((uint16_t)rx[3] << 8) | (uint16_t)rx[4];

    *temp = -45.0f + 175.0f * (float)raw_temp / 65535.0f;
    *hum  = 100.0f * (float)raw_hum / 65535.0f;
    return 1;
}

/********************************************************************************
* 名    称：BH1750_Read
* 功    能：读取光照强度
* 参数说明：lux：光照强度，单位 lx
* 返 回 值：1：读取成功  0：读取失败
* 说    明：先发上电命令 0x01，再发连续高分辨率模式 0x10，180ms 后读 2 字节
* 调用方法：内部函数
********************************************************************************/
static uint8_t BH1750_Read(uint16_t *lux)
{
    uint8_t cmd = 0;
    uint8_t *rx = NULL;
    uint16_t raw = 0;

    cmd = 0x01;                                  /* 上电 */
    if(!Env_I2C_Write(BH1750_ADDR_W, &cmd, 1))
    {
        return 0;
    }

    cmd = 0x10;                                  /* 连续高分辨率模式，分辨率 1lx */
    if(!Env_I2C_Write(BH1750_ADDR_W, &cmd, 1))
    {
        return 0;
    }
    HAL_Delay(180);

    rx = Env_I2C_Read(BH1750_ADDR_R, 2);
    if(rx == NULL)
    {
        return 0;
    }

    raw = ((uint16_t)rx[0] << 8) | (uint16_t)rx[1];
    *lux = (uint16_t)((float)raw / 1.2f);
    return 1;
}

/********************************************************************************
* 名    称：SGP30_Write_Cmd
* 功    能：向 SGP30 发送 2 字节命令
* 参数说明：cmd：命令高字节   arg：命令低字节
* 返 回 值：1：发送完成  0：参数错误
* 说    明：无
* 调用方法：内部函数
********************************************************************************/
static uint8_t SGP30_Write_Cmd(uint8_t cmd, uint8_t arg)
{
    uint8_t buf[2];

    buf[0] = cmd;
    buf[1] = arg;
    return Env_I2C_Write(SGP30_ADDR_W, buf, 2);
}

/********************************************************************************
* 名    称：SGP30_Init
* 功    能：SGP30 初始化
* 参数说明：无
* 返 回 值：无
* 说    明：发送 0x20 0x03 初始化空气质量测量
* 调用方法：内部函数
********************************************************************************/
static void SGP30_Init(void)
{
    SGP30_Write_Cmd(0x20, 0x03);
    HAL_Delay(20);
}

/********************************************************************************
* 名    称：SGP30_Read
* 功    能：读取二氧化碳、TVOC 值
* 参数说明：co2：二氧化碳，单位 ppm   tvoc：总挥发性有机物，单位 ppb
* 返 回 值：1：读取成功  0：读取失败
* 说    明：发送 0x20 0x08 启动测量，30ms 后读 6 字节（2 组数据各带 1 字节 CRC）
* 调用方法：内部函数
********************************************************************************/
static uint8_t SGP30_Read(uint16_t *co2, uint16_t *tvoc)
{
    uint8_t *rx = NULL;

    if(!SGP30_Write_Cmd(0x20, 0x08))
    {
        return 0;
    }
    HAL_Delay(30);

    rx = Env_I2C_Read(SGP30_ADDR_R, 6);
    if(rx == NULL)
    {
        return 0;
    }

    if((SHT30_Crc8(&rx[0], 2) != rx[2]) || (SHT30_Crc8(&rx[3], 2) != rx[5]))
    {
        return 0;
    }

    *co2  = ((uint16_t)rx[0] << 8) | (uint16_t)rx[1];
    *tvoc = ((uint16_t)rx[3] << 8) | (uint16_t)rx[4];
    return 1;
}

/********************************************************************************
* 名    称：Env_Sensor_Init
* 功    能：传感器初始化
* 参数说明：无
* 返 回 值：无
* 说    明：打开 I2C2（PB13=SCL，PB14=SDA），让 BH1750 上电、SGP30 进入测量状态
* 调用方法：外部函数
********************************************************************************/
void Env_Sensor_Init(void)
{
    uint8_t cmd = 0;

    memset(&g_env_data, 0, sizeof(g_env_data));

    i2c_open(ENV_I2C_PORT);
    HAL_Delay(10);

    cmd = 0x01;                                  /* BH1750 上电 */
    Env_I2C_Write(BH1750_ADDR_W, &cmd, 1);
    HAL_Delay(10);

    SGP30_Init();
}

/********************************************************************************
* 名    称：Env_Sensor_Read_All
* 功    能：依次读取三个传感器
* 参数说明：无
* 返 回 值：无
* 说    明：每路读取结果由对应的 ok 标志指示
* 调用方法：外部函数
********************************************************************************/
void Env_Sensor_Read_All(void)
{
    g_env_data.sht30_ok  = SHT30_Read(&g_env_data.temperature, &g_env_data.humidity);
    g_env_data.bh1750_ok = BH1750_Read(&g_env_data.light);
    g_env_data.cps121_ok = CPS121_Read(&g_env_data.pressure);
    g_env_data.sgp30_ok  = SGP30_Read(&g_env_data.co2, &g_env_data.tvoc);
}
