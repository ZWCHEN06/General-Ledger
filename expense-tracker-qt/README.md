# expense-tracker-qt

Qt 6 + Qt Quick + QML + CMake 的个人记账 App 桌面版项目骨架。

第一版当前只包含最小可运行窗口，窗口中显示“记账 App”。

## 构建

```bash
cmake -S . -B build
cmake --build build
```

## 运行

```bash
./build/expense-tracker-qt
```

当前项目不包含数据库代码，也不包含业务逻辑。

