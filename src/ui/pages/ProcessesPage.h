#pragma once

#include <QWidget>
#include <QLabel>
#include <vector>

#include "../../core/Types.h"
#include "../components/ProcessTableView.h"

namespace pulse::ui {

class ProcessesPage : public QWidget {
    Q_OBJECT

public:
    explicit ProcessesPage(QWidget* parent = nullptr);
    ~ProcessesPage() override = default;

    void updateProcesses(const std::vector<ProcessInfo>& processes);

private slots:
    void onProcessSelected(const ProcessInfo& process);

private:
    void setupUi();

    QLabel* m_summaryLabel{nullptr};
    ProcessTableView* m_processTable{nullptr};

    // Details panel
    QWidget* m_detailsPanel{nullptr};
    QLabel* m_detailNameLabel{nullptr};
    QLabel* m_detailPidLabel{nullptr};
    QLabel* m_detailPathLabel{nullptr};
    QLabel* m_detailCpuLabel{nullptr};
    QLabel* m_detailRamLabel{nullptr};
    QLabel* m_detailThreadsLabel{nullptr};
    QLabel* m_detailPrioLabel{nullptr};
    QLabel* m_detailAccessLabel{nullptr};
};

} // namespace pulse::ui

