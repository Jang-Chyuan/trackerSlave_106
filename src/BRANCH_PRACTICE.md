# Git 分支協作練習

這份練習使用 `GIT_test` 專案，模擬不同協作者在各自分支完成工作，再將成果合併回 `main`。

## 分支用途

- `main`：穩定、正式版本。
- `feature/task1-led`：協作者 A 開發 Task 1。
- `feature/task2-status`：協作者 B 開發 Task 2。
- `fix/serial-error`：修正序列埠問題。
- `docs/git-guide`：修改說明文件。

每個分支都能擁有自己的 commit 紀錄和檔案內容。切換分支後，VS Code 顯示的檔案也會切換到該分支的版本。

## 開始前檢查

在 VS Code 終端機執行：

```powershell
cd C:\Dog_gps\GIT_test
git status
git branch --all
```

請確認目前位於 `main`，而且顯示 `working tree clean`。如果有尚未提交的修改，先 commit 再繼續。

## 練習一：協作者 A 開發 Task 1

從 `main` 建立分支：

```powershell
git switch main
git switch -c feature/task1-led
```

開啟 `src/task1.cpp`，將：

```cpp
constexpr uint32_t BLINK_INTERVAL_MS = 500;
```

改成：

```cpp
constexpr uint32_t BLINK_INTERVAL_MS = 1000;
```

查看並提交修改：

```powershell
git status
git diff
git add src/task1.cpp
git commit -m "feat: slow task1 LED blink interval"
```

## 練習二：協作者 B 開發 Task 2

先切回 `main`。注意：此時 `task1.cpp` 會恢復成 `main` 中的 500 ms，因為 Task 1 的修改只存在另一個分支。

```powershell
git switch main
git switch -c feature/task2-status
```

開啟 `src/task2.cpp`，將：

```cpp
constexpr uint32_t REPORT_INTERVAL_MS = 2000;
```

改成：

```cpp
constexpr uint32_t REPORT_INTERVAL_MS = 3000;
```

提交修改：

```powershell
git diff
git add src/task2.cpp
git commit -m "feat: change task2 report interval"
```

## 比較分支

查看所有分支與 commit 關係：

```powershell
git log --oneline --graph --decorate --all
```

比較兩個功能分支：

```powershell
git diff feature/task1-led..feature/task2-status
```

確認目前所在分支：

```powershell
git branch
```

分支名稱前的 `*` 表示目前所在的分支。

## 將兩個功能合併回 main

先合併 Task 1：

```powershell
git switch main
git merge feature/task1-led
```

再合併 Task 2：

```powershell
git merge feature/task2-status
```

由於兩位協作者修改不同檔案，通常可以自動合併。檢查結果：

```powershell
git status
git log --oneline --graph --decorate --all
```

## 練習三：修正序列埠文字

建立修正分支：

```powershell
git switch main
git switch -c fix/serial-error
```

修改 `src/main.cpp` 中的啟動訊息，例如：

```cpp
Serial.println("Starting Git branch practice project...");
```

提交並合併：

```powershell
git add src/main.cpp
git commit -m "fix: clarify serial startup message"
git switch main
git merge fix/serial-error
```

## 練習四：只修改文件

建立文件分支：

```powershell
git switch main
git switch -c docs/git-guide
```

在 `GIT_BEGINNER_GUIDE.md` 增加一段學習筆記，然後提交：

```powershell
git add GIT_BEGINNER_GUIDE.md
git commit -m "docs: add branch learning notes"
git switch main
git merge docs/git-guide
```

## 編譯與清理分支

合併完成後編譯正式版本：

```powershell
pio run
```

確認功能分支均已合併後，可以刪除本機分支：

```powershell
git branch -d feature/task1-led
git branch -d feature/task2-status
git branch -d fix/serial-error
git branch -d docs/git-guide
```

最後查看：

```powershell
git branch --all
git status
```

預期只留下 `main`，且工作目錄為乾淨狀態。

## 多人網路協作時的差異

真正多人協作時，每位協作者將自己的分支推送到 GitHub：

```powershell
git push -u origin feature/task1-led
```

接著在 GitHub 建立 Pull Request，由其他人 review 後合併至 `main`。不要讓每位協作者直接修改遠端 `main`。
