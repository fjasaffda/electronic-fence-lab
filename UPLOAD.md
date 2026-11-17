# 上传导引（Gitee / GitHub）

两个平台都已完成，本文件记录最终位置、以及以后怎么更新。

## 一、当前状态

| 平台 | 状态 | 地址 | 可见性 |
| --- | --- | --- | --- |
| Gitee | ✅ 已建仓并推送 | <https://gitee.com/mywuyu/electronic-fence-lab> | 私有（可一键改公开） |
| GitHub | ✅ 已建仓并上传 | <https://github.com/fjasaffda/electronic-fence-lab> | 公开 |

* 仓库名：`electronic-fence-lab`，默认分支 `main`
* 文件数：825（源码 + 文档 + 说明 PDF），已排除 299 MB 编译中间产物
* 固件里的 WiFi 密码与设备密钥已替换为 `YOUR_WIFI_PASSWORD` / `YOUR_DEVICE_SECRET`

## 二、Gitee 改公开

仓库是私有的，别人打不开。公开方法：

1. 打开 <https://gitee.com/mywuyu/electronic-fence-lab>
2. 右侧「管理」→「基本信息」
3. 「是否公开」选 **公开** → 保存

## 三、以后怎么更新

### Gitee（HTTPS 正常）

```powershell
git push gitee main
```

### GitHub（本机直连 github.com 不通）

这台机器上 `github.com` 的 HTTPS 连不上（DNS 解析到 20.6.7.17，TLS 握手超时），
但 `api.github.com` 通。所以 GitHub 侧不用 `git push`，改用 Git Data API 上传：

```powershell
$env:GH_TOKEN = "<你的 PAT>"
node scripts/gh-upload.js . fjasaffda electronic-fence-lab main "提交说明"
```

脚本会：按 `git ls-files` 枚举文件 → 逐个建 blob → 建 tree → 建 commit（父提交取远端 `main` 头）→ 移动分支指针。
需要 PAT 带 `repo` 权限（<https://github.com/settings/tokens>）。

### 如果能连上 github.com

那就常规做法，先加远端再推：

```powershell
git remote add github https://github.com/fjasaffda/electronic-fence-lab.git
git push -u github main
```

## 四、完全不脚本的手动路线

桌面上的两个产物，网页上传即可：

| 文件 | 用途 |
| --- | --- |
| `electronic-fence-lab-source.zip` | 全部源码（约 3.7 MB），Gitee、GitHub 网页「上传文件」都能直接吃 |
| `electronic-fence-lab.bundle` | git 完整包，`git clone electronic-fence-lab.bundle` 可还原全部提交 |

## 五、仓库里都有什么

* `ZHIoT_Farm_Fence/` —— 电子围栏工程（STM32L431 + LiteOS + ESP8285 + 华为 OC）
* `ZHIoT_Sensor_Env/` —— 环境传感器工程（CPS121 / SHT30 / BH1750 / SGP30，共用 I2C2）
* `keilkilll.bat` —— 清理 Keil 编译中间产物
* `.gitignore` —— 已排除 299 MB 编译中间产物
* `README.md` —— 结构、编译烧录、硬件连线、任务划分、云端字段
* `UPLOAD.md` —— 本文件
* `scripts/push-all.ps1` —— Gitee / GitHub 一键建仓 + 推送（HTTPS 直连可用时）
* `scripts/gh-upload.js` —— 走 api.github.com 的 Git Data API 上传器（github.com 不通时用这个）

## 六、安全提示

令牌是明文口令级凭据，推送完成后建议吊销：

* Gitee：<https://gitee.com/profile/personal_access_tokens>
* GitHub：<https://github.com/settings/tokens>