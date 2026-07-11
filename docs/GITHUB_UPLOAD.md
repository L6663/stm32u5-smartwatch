# 上传到 GitHub

## 方式一：PowerShell 脚本

在仓库根目录打开 PowerShell：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\push_to_github.ps1 \
  -RepositoryUrl "https://github.com/你的用户名/你的仓库.git" \
  -Branch main
```

脚本会执行仓库检查、初始化 Git、创建首次提交并推送。GitHub 登录和授权仍由本机 Git 完成。

## 方式二：手动命令

```bash
python scripts/verify_repo.py

git init
git branch -M main
git add .
git commit -m "chore: import STM32U5 smartwatch alpha source"
git remote add origin https://github.com/USER/REPOSITORY.git
git push -u origin main
```

## 注意

- 先解压，再上传目录；不要只把 ZIP 当成仓库源码上传。
- 本仓库最大单文件远低于 GitHub 100 MB 限制，不需要 Git LFS。
- `.gitignore` 已排除 Keil 构建输出，后续不要强制提交 `.axf/.hex/.map`。
- 若目标仓库已有 README，首次推送前先确认是否需要合并远端历史。
