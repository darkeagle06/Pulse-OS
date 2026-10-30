#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QProgressBar>
#include <memory>
#include <vector>
#include <optional>

#include "../../core/Types.h"
#include "../../core/InstalledAppManager.h"

namespace pulse::ui {

class ApplicationsPage : public QWidget {
    Q_OBJECT

public:
    explicit ApplicationsPage(std::shared_ptr<InstalledAppManager> appManager = nullptr,
                             QWidget* parent = nullptr);
    ~ApplicationsPage() override = default;

    void updateProcessContext(const std::vector<ProcessInfo>& processes);
    void triggerScan();

signals:
    void scanCompleted(size_t totalApps, size_t unusedApps);

private slots:
    void onSearchChanged(const QString& query);
    void onSortChanged(int index);
    void onCategorySelected(int categoryIndex);
    void onTableSelectionChanged();
    void onUninstallClicked();
    void onScanFinished(const std::vector<InstalledAppInfo>& apps);

private:
    void setupUi();
    void applyFilterAndSort();
    void populateTable();
    void updateDetailsPanel(const InstalledAppInfo* app);
    void updateStatCards();

    std::shared_ptr<InstalledAppManager> m_appManager;
    std::vector<InstalledAppInfo> m_allApps;
    std::vector<InstalledAppInfo> m_displayedApps;
    std::vector<ProcessInfo> m_cachedProcesses;

    std::optional<AppCategory> m_activeCategory;
    InstalledAppManager::SortField m_activeSort{InstalledAppManager::SortField::NameAsc};
    QString m_searchQuery;
    int m_selectedRow{-1};
    bool m_isScanning{false};

    // Header & Stats
    QLabel* m_statusPill{nullptr};
    QLabel* m_totalCountVal{nullptr};
    QLabel* m_totalSizeVal{nullptr};
    QLabel* m_unusedCountVal{nullptr};
    QLabel* m_largeCountVal{nullptr};

    // Filter controls
    QPushButton* m_btnFilterAll{nullptr};
    QPushButton* m_btnFilterUnused{nullptr};
    QPushButton* m_btnFilterLarge{nullptr};
    QPushButton* m_btnFilterRecent{nullptr};
    QLineEdit* m_searchBox{nullptr};
    QComboBox* m_sortCombo{nullptr};
    QPushButton* m_refreshBtn{nullptr};
    QProgressBar* m_scanProgressBar{nullptr};

    // Main Table
    QTableWidget* m_table{nullptr};

    // Details Inspector
    QWidget* m_detailsPanel{nullptr};
    QLabel* m_detailName{nullptr};
    QLabel* m_detailPublisher{nullptr};
    QLabel* m_detailVersion{nullptr};
    QLabel* m_detailSize{nullptr};
    QLabel* m_detailDate{nullptr};
    QLabel* m_detailLocation{nullptr};
    QLabel* m_detailEvidence{nullptr};
    QPushButton* m_uninstallBtn{nullptr};
    QLabel* m_actionStatusLabel{nullptr};
};

} // namespace pulse::ui

