#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <vector>

#include "../../core/Types.h"
#include "../../core/ProcessMonitor.h"

namespace pulse::ui {

class ProcessTableView : public QWidget {
    Q_OBJECT

public:
    explicit ProcessTableView(QWidget* parent = nullptr);
    ~ProcessTableView() override = default;

    void updateProcesses(const std::vector<ProcessInfo>& processes);
    [[nodiscard]] std::optional<ProcessInfo> getSelectedProcess() const;

signals:
    void processSelected(const ProcessInfo& process);

private slots:
    void onSearchTextChanged(const QString& text);
    void onSortOptionChanged(int index);
    void onCellClicked(int row, int column);

private:
    void setupUi();
    void renderTable();
    static QString formatMemory(uint64_t bytes);

    QLineEdit* m_searchBox{nullptr};
    QComboBox* m_sortCombo{nullptr};
    QTableWidget* m_table{nullptr};

    std::vector<ProcessInfo> m_allProcesses;
    std::vector<ProcessInfo> m_displayedProcesses;
    ProcessMonitor::SortField m_currentSort{ProcessMonitor::SortField::CpuDesc};
    QString m_currentQuery;
};

} // namespace pulse::ui

