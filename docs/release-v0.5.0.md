# v0.5.0 二级分类功能版本说明

## 版本名称

v0.5.0 - 二级分类功能

## 新增功能

- 在已有一级分类下新增二级分类体系，支持收入和支出二级分类。
- 支持默认二级分类初始化，并支持用户自定义二级分类。
- 支持按一级分类进入二级分类管理页，进行新增、编辑、删除。
- 新增账单支持选择“一级分类 / 二级分类”。
- 编辑账单支持回显和修改二级分类。
- 账单列表和首页最近账单支持显示“一级分类 / 二级分类”。
- CSV 导出增加二级分类列。
- 搜索筛选支持按二级分类筛选。
- 周预算继续按一级分类统计，不引入二级分类预算。

## 用户使用流程

1. 进入分类管理页。
2. 在某个一级分类项上点击“二级分类”。
3. 进入二级分类管理页后，可以查看该一级分类下的默认二级分类。
4. 用户可以新增自定义二级分类。
5. 用户可以编辑或删除未被账单使用的自定义二级分类。
6. 默认二级分类不能编辑或删除。
7. 新增账单时先选择收入/支出类型，再选择一级分类，然后选择该一级分类下的二级分类。
8. 编辑账单时页面会回显原账单的一级分类和二级分类，用户可以修改后保存。
9. 在账单列表中，有二级分类的账单显示为“一级分类 / 二级分类”，旧账单无二级分类时只显示一级分类。
10. 在筛选面板中，选择一级分类后，二级分类筛选项只显示该一级分类下的数据。

## 数据库变化

- 新增 `subcategories` 表，用于保存二级分类。
- `subcategories.category_id` 指向 `categories.id`，表示二级分类所属一级分类。
- `subcategories` 使用 `UNIQUE(category_id, name)` 保证同一一级分类下二级分类名称不重复。
- `transactions` 表新增 `subcategory_id` 字段。
- `transactions` 表新增 `subcategory` 文本快照字段。
- `subcategory_id` 和 `subcategory` 允许为空，用于兼容旧账单。
- `weekly_budgets` 表结构未修改。

## 新增 C++ 类

- `Subcategory`
  - 二级分类领域模型。
  - 字段包括 `id`、`categoryId`、`name`、`isDefault`、`sortOrder`、`createdAt`、`updatedAt`。
  - 提供基础 getter、setter 和中文校验信息。

- `SubcategoryRepository`
  - 负责二级分类数据库读写。
  - 支持按一级分类查询、按 id 查询、按名称和一级分类查询。
  - 支持新增、改名、删除二级分类。
  - 删除时会检查默认分类和账单占用情况。

- `SubcategoryListModel`
  - 提供给 QML 使用的二级分类列表模型。
  - 支持 `refresh(categoryId)` 和 `clear()`。
  - roles 包括 `id`、`categoryId`、`name`、`isDefault`、`sortOrder`。

## 修改的 C++ 类

- `DatabaseManager`
  - 增加 schema version 6 到 9 的二级分类相关 migration。
  - 创建 `subcategories` 表。
  - 为 `transactions` 增加 `subcategory_id` 和 `subcategory` 字段。
  - 初始化默认二级分类。
  - 增加二级分类和分类筛选相关索引。

- `Transaction`
  - 增加 `subcategoryId` 和 `subcategory` 字段。

- `TransactionRepository`
  - 查询账单时读取二级分类字段。
  - 新增账单时写入 `subcategory_id` 和 `subcategory`。
  - 编辑账单时更新 `subcategory_id` 和 `subcategory`。
  - 筛选逻辑支持按 `subcategory_id` 查询。
  - 一级分类筛选优先使用 `category_id`，并保留旧账单文本兜底。

- `TransactionFilter`
  - 增加一级分类 id 和二级分类 id 筛选条件。

- `AppController`
  - 暴露二级分类列表模型和管理接口给 QML。
  - 新增、编辑账单接口支持传入二级分类。
  - 搜索筛选接口支持一级分类 id 和二级分类 id。

- `CsvExportService`
  - CSV 表头支持“一级分类”和“二级分类”。
  - 旧账单无二级分类时导出空字符串。

## 新增 QML 页面

- `SubcategoryManagePage.qml`
  - 二级分类管理页面。
  - 接收一级分类 id 和一级分类名称。
  - 显示当前一级分类下的二级分类。
  - 支持新增、编辑、删除自定义二级分类。
  - Android 小屏幕可滚动。

## 修改的 QML 页面

- `CategoryManagePage.qml`
  - 一级分类项增加“二级分类”入口。
  - 点击后进入 `SubcategoryManagePage.qml`。

- `AddTransactionPage.qml`
  - 新增二级分类选择区域。
  - 切换一级分类时刷新二级分类列表。
  - 默认选中当前一级分类下第一个二级分类。
  - 保存时传入一级分类和二级分类 id/name。

- `EditTransactionPage.qml`
  - 加载账单时回显一级分类和二级分类。
  - 支持按 `subcategory_id` 回显，旧数据可按文本兜底。
  - 切换一级分类时重置二级分类。
  - 旧账单没有二级分类时不会崩溃。

- `TransactionListPage.qml`
  - 账单列表显示“一级分类 / 二级分类”。
  - 筛选面板支持选择一级分类和二级分类。
  - 只选择一级分类时显示该一级分类下全部账单。
  - 选择二级分类时只显示对应二级分类账单。

- 首页相关 QML
  - 最近账单显示规则与账单列表保持一致。

## Migration 说明

- 当前数据库 schema 版本为 9。
- v6 migration：
  - 创建 `subcategories` 表。
- v7 migration：
  - 给 `transactions` 表新增 `subcategory_id`。
  - 给 `transactions` 表新增 `subcategory`。
- v8 migration：
  - 初始化默认二级分类。
  - 使用 `INSERT OR IGNORE` 避免重复插入。
  - 如果某个一级分类不存在，会跳过对应二级分类并记录日志。
- v9 migration：
  - 新增 `idx_subcategories_category_sort`。
  - 新增 `idx_transactions_subcategory`。
  - 新增 `idx_transactions_category`。

迁移原则：

- 不删除已有字段。
- 不修改已有账单数据。
- 二级分类字段允许为空以兼容旧账单。
- migration 失败时返回明确错误并中止升级。

## 与周预算的兼容说明

- `weekly_budgets` 表结构保持不变。
- 周预算仍然绑定一级分类 `category_id`。
- 实际支出统计仍按一级分类汇总。
- 同一一级分类下的不同二级分类支出会合并计入该一级分类预算。
- 收入不参与预算统计。
- v0.5.0 不提供二级分类预算功能。

示例：

- 餐饮周预算：300。
- 餐饮 / 早餐支出：50。
- 餐饮 / 午餐支出：80。
- 餐饮实际支出应为：130。

## 手动测试结果

手动验收清单已生成：

- `docs/test-v0.5.0-subcategories.md`

当前建议验收范围：

- 数据库 migration 与旧数据兼容。
- 默认二级分类初始化与重复启动去重。
- 二级分类新增、编辑、删除。
- 新增账单选择二级分类。
- 编辑账单回显和修改二级分类。
- 旧账单无二级分类时兼容显示。
- 账单列表和首页显示“一级 / 二级”。
- CSV 导出二级分类列。
- 搜索筛选按一级分类和二级分类工作。
- 周预算继续按一级分类统计。
- Android 小屏幕页面可用性。

当前代码层验证：

- C++/QML 已通过本地构建验证。
- P1 和 P2 审计问题已完成修复并推送到 `main`。

仍需人工确认：

- Android 真机或模拟器全流程验收。
- 使用真实旧数据库执行升级回归。
- CSV 文件在目标表格软件中的打开效果。

## 已知问题

- QML 中新增账单、编辑账单、筛选面板存在部分相似的分类选择逻辑，后续可抽取复用组件降低维护成本。
- Android 不同屏幕尺寸下仍需要真机或模拟器做完整交互确认。
- 旧账单如果只有二级分类文本快照、没有 `subcategory_id`，编辑时依赖文本兜底匹配；如果同名数据被用户调整，可能需要用户重新选择二级分类。
- v0.5.0 暂不支持二级分类预算。
- v0.5.0 暂不支持二级分类排序管理界面。

## 后续计划

- 抽取可复用的一级/二级分类选择 QML 组件。
- 增加二级分类排序管理能力。
- 增加更完整的自动化测试覆盖，包括 Repository、migration 和筛选组合。
- 补充 Android 真机验收记录。
- 评估是否需要二级分类维度的统计图表。
- 评估是否在未来版本支持二级分类预算。
