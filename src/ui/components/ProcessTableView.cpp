#include "ProcessTableView.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>

namespace pulse::ui {

ProcessTableView::ProcessTableView(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void ProcessTableView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    // Controls bar: Search + Sort
    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);

    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search by process name or PID...");
    m_searchBox->setClearButtonEnabled(true);
    controlsLayout->addWidget(m_searchBox, 1);

    auto* sortLabel = new QLabel("Sort by:", this);
    sortLabel->setStyleSheet(QString("color: %1; font-weight: 500;").arg(Theme::TextSecondary));
    controlsLayout->addWidget(sortLabel);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem("CPU Usage (Highest First)", static_cast<int>(ProcessMonitor::SortField::CpuDesc));
    m_sortCombo->addItem("Memory Usage (Highest First)", static_cast<int>(ProcessMonitor::SortField::MemoryDesc));
    m_sortCombo->addItem("Process Name (A-Z)", static_cast<int>(ProcessMonitor::SortField::NameAsc));
    m_sortCombo->addItem("PID (Ascending)", static_cast<int>(ProcessMonitor::SortField::PidAsc));
    controlsLayout->addWidget(m_sortCombo);

    layout->addLayout(controlsLayout);

    // Table Widget
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "PID", "Process Name", "CPU %", "RAM Usage", "Threads", "Priority", "Access Status"
    });

    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 75);
    m_table->setColumnWidth(2, 85);
    m_table->setColumnWidth(3, 110);
    m_table->setColumnWidth(4, 75);
    m_table->setColumnWidth(5, 95);
    m_table->setColumnWidth(6, 160);

    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);

    layout->addWidget(m_table);

    // Connections
    connect(m_searchBox, &QLineEdit::textChanged, this, &ProcessTableView::onSearchTextChanged);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProcessTableView::onSortOptionChanged);
    connect(m_table, &QTableWidget::cellClicked, this, &ProcessTableView::onCellClicked);
}

QString ProcessTableView::formatMemory(uint64_t bytes) {
    if (bytes == 0) return "--";
    double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    if (mb >= 1024.0) {
        return QString::number(mb / 1024.0, 'f', 2) + " GB";
    }
    return QString::number(mb, 'f', 1) + " MB";
}

void ProcessTableView::updateProcesses(const std::vector<ProcessInfo>& processes) {
    m_allProcesses = processes;
    renderTable();
}

void ProcessTableView::renderTable() {
    m_table->setUpdatesEnabled(false);

    // 1. Filter
    m_displayedProcesses = ProcessMonitor::filterByName(m_allProcesses, m_currentQuery.toStdString());

    // 2. Sort
    ProcessMonitor::sortProcesses(m_displayedProcesses, m_currentSort);

    // 3. Populate
    m_table->setRowCount(static_cast<int>(m_displayedProcesses.size()));

    for (int r = 0; r < static_cast<int>(m_displayedProcesses.size()); ++r) {
        const auto& p = m_displayedProcesses[r];

        // PID
        auto* pidItem = new QTableWidgetItem(QString::number(p.pid));
        pidItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(r, 0, pidItem);

        // Name
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(p.name));
        nameItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        nameItem->setToolTip(QString::fromStdString(p.executablePath.empty() ? p.name : p.executablePath));
        m_table->setItem(r, 1, nameItem);

        // CPU %
        auto* cpuItem = new QTableWidgetItem(QString::number(p.cpuUsagePercent, 'f', 1) + "%");
        cpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (p.cpuUsagePercent >= 10.0) {
            cpuItem->setForeground(Theme::colorAccentRed());
            cpuItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        } else if (p.cpuUsagePercent >= 3.0) {
            cpuItem->setForeground(Theme::colorAccentAmber());
        }
        m_table->setItem(r, 2, cpuItem);

        // RAM Usage
        auto* ramItem = new QTableWidgetItem(formatMemory(p.workingSetBytes));
        ramItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(r, 3, ramItem);

        // Threads
        auto* threadItem = new QTableWidgetItem(QString::number(p.threadCount));
        threadItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(r, 4, threadItem);

        // Priority
        auto* prioItem = new QTableWidgetItem(QString::fromStdString(p.priorityClassString));
        prioItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(r, 5, prioItem);

        // Access Status
        QString statusStr;
        QColor statusColor;
        switch (p.accessStatus) {
            case ProcessAccessStatus::Accessible:
                statusStr = "Accessible";
                statusColor = Theme::colorAccentGreen();
                break;
            case ProcessAccessStatus::AccessDenied:
                statusStr = "Restricted (Access Denied)";
                statusColor = Theme::colorAccentAmber();
                break;
            case ProcessAccessStatus::SystemProtected:
                statusStr = "System Protected";
                statusColor = QColor(Theme::TextMuted);
                break;
            default:
                statusStr = "Unknown";
                statusColor = QColor(Theme::TextMuted);
                break;
        }

        auto* statusItem = new QTableWidgetItem(statusStr);
        statusItem->setForeground(statusColor);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(r, 6, statusItem);
    }

    m_table->setUpdatesEnabled(true);
}

void ProcessTableView::onSearchTextChanged(const QString& text) {
    m_currentQuery = text;
    renderTable();
}

void ProcessTableView::onSortOptionChanged(int index) {
    m_currentSort = static_cast<ProcessMonitor::SortField>(
        m_sortCombo->itemData(index).toInt());
    renderTable();
}

void ProcessTableView::onCellClicked(int row, int /*column*/) {
    if (row >= 0 && row < static_cast<int>(m_displayedProcesses.size())) {
        emit processSelected(m_displayedProcesses[row]);
    }
}

std::optional<ProcessInfo> ProcessTableView::getSelectedProcess() const {
    int row = m_table->currentRow();
    if (row >= 0 && row < static_cast<int>(m_displayedProcesses.size())) {
        return m_displayedProcesses[row];
    }
    return std::nullopt;
}

} // namespace pulse::ui

