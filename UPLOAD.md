# 上传导引（Gitee / GitHub）

本文件记录这份教学工程已经上网的位置，以及后续怎么补齐另一个平台。

## 一、当前状态

| 平台 | 状态 | 位置 |
| --- | --- | --- |
| Gitee | ✅ 已建仓并推送完成 | `https://gitee.com/mywuyu/electronic-fence-lab` |
| GitHub | ⏳ 待推送（缺 GitHub 个人访问令牌） | — |

* 仓库名：`electronic-fence-lab`
* 默认分支：`main`，最新提交 `ead37da`
* 可见性：**私有**（Gitee API 建仓时该账号默认落私有，网页端一键可改公开，见第二节）
* 令牌已从 `.git/config` 的 remote URL 里摘掉，配置里只剩干净的 HTTPS 地址

## 二、把 Gitee 仓库改成公开

仓库是私有的，别人打不开。想公开：

1. 打开 <https://gitee.com/mywuyu/electronic-fence-lab>
2. 右侧「管理」→「基本信息」
3. 「是否公开」选 **公开** → 保存

私有状态下用令牌或登录态访问一切正常，只是不能被匿名访问。

## 三、GitHub 还差什么

GitHub 上没有任何可用认证材料（无 `gh` CLI、无 SSH 密钥、无 `~/.netrc`、无 `GH_TOKEN`、凭据管理器无 github 条目），所以建仓这步必须给一个 PAT。

1. 打开 <https://github.com/settings/tokens> → `Generate new token (classic)`
2. 勾 `repo`，生成后复制 `ghp_` 开头的那串
3. 在仓库根目录执行：

```powershell
.\scripts\push-all.ps1 -GitHubToken "<GitHub PAT>" -GitHubOwner "<你的GitHub用户名>"
```

脚本会 `POST /user/repos` 建仓，然后推送 `main`，最后把令牌从 remote URL 摘掉。

## 四、完全不脚本的手动路线

桌面上已有两个产物，网页上传即可：

| 文件 | 用途 |
| --- | --- |
| `electronic-fence-lab-source.zip` | 全部源码（约 3.6 MB），Gitee、GitHub 网页「上传文件」都能直接吃 |
| `electronic-fence-lab.bundle` | git 完整包，`git clone electronic-fence-lab.bundle` 可还原全部提交 |

GitHub 网页上传：新建仓库 → `Add file` → `Upload files` → 拖入解压后的文件。

## 五、仓库里都有什么

* `ZHIoT_Farm_Fence/` —— 电子围栏工程（STM32L431 + LiteOS + ESP8285 + 华为 OC）
* `ZHIoT_Sensor_Env/` —— 环境传感器工程（CPS121 / SHT30 / BH1750 / SGP30，共用 I2C2）
* `keilkilll.bat` —— 清理 Keil 编译中间产物
* `.gitignore` —— 已排除 299 MB 编译中间产物
* `README.md` —— 结构、编译烧录、硬件连线、任务划分、云端字段
* `UPLOAD.md` —— 本文件
* `scripts/push-all.ps1` —— 一键建仓 + 推送（Gitee / GitHub 都支持）

固件里原来的 WiFi 密码与设备密钥已替换为占位符（`YOUR_WIFI_PASSWORD` / `YOUR_DEVICE_SECRET`），服务器地址、产品 ID、时间戳等实验参数保持原样。

## 六、安全提示

`push-all.ps1` 用令牌推送后会立刻 `git remote set-url` 把令牌抹掉；但令牌本身仍是明文口令级凭据，用完建议到 <https://gitee.com/profile/personal_access_tokens> 吊销。