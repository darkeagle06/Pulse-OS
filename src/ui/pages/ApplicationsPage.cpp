#include "ApplicationsPage.h"
#include "../Theme.h"
#include "../components/UninstallConfirmDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QThreadPool>
#include <QMessageBox>
#include <QDateTime>

namespace pulse::ui {

ApplicationsPage::ApplicationsPage(std::shared_ptr<InstalledAppManager> appManager, QWidget* parent)
    : QWidget(parent),
      m_appManager(appManager ? std::move(appManager) : std::make_shared<InstalledAppManager>()) {
    setupUi();
    triggerScan();
}

void ApplicationsPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // 1. Header Bar
    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("Installed Application Manager", this);
    titleLabel->setStyleSheet(Theme::pageTitleStyleSheet());
    headerLayout->addWidget(titleLabel);

    m_statusPill = new QLabel("Scanning registry...", this);
    m_statusPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::TextSecondary));
    headerLayout->addWidget(m_statusPill, 0, Qt::AlignRight);

    mainLayout->addLayout(headerLayout);

    // 2. Summary Stat Cards (4 cards in a row)
    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(12);

    auto createStatCard = [this](const QString& title, QLabel*& valLabel, const QString& accentColor) -> QFrame* {
        auto* card = new QFrame(this);
        card->setStyleSheet(Theme::panelStyleSheet(8));
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setMinimumHeight(76);

        auto* l = new QVBoxLayout(card);
        l->setContentsMargins(14, 12, 14, 12);
        l->setSpacing(4);

        auto* tLabel = new QLabel(title, card);
        tLabel->setStyleSheet(QString("font-size: 11px; font-weight: 600; text-transform: uppercase; color: %1;").arg(Theme::TextSecondary));
        l->addWidget(tLabel);

        valLabel = new QLabel("--", card);
        valLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(accentColor));
        l->addWidget(valLabel);

        return card;
    };

    statsLayout->addWidget(createStatCard("Total Installed", m_totalCountVal, Theme::TextPrimary));
    statsLayout->addWidget(createStatCard("Estimated Disk Use", m_totalSizeVal, Theme::AccentCyan));
    statsLayout->addWidget(createStatCard("Potentially Unused", m_unusedCountVal, Theme::AccentAmber));
    statsLayout->addWidget(createStatCard("Large Apps (>500MB)", m_largeCountVal, Theme::AccentPurple));

    mainLayout->addLayout(statsLayout);

    // Progress bar for scanning
    m_scanProgressBar = new QProgressBar(this);
    m_scanProgressBar->setRange(0, 0); // Indeterminate
    m_scanProgressBar->setFixedHeight(3);
    m_scanProgressBar->setTextVisible(false);
    m_scanProgressBar->setStyleSheet(QString("QProgressBar { background: %1; border: none; } QProgressBar::chunk { background: %2; }")
                                         .arg(Theme::BgSurface)
                                         .arg(Theme::AccentCyan));
    mainLayout->addWidget(m_scanProgressBar);

    // 3. Category Filter Pills & Search & Sort Controls
    auto* filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(8);

    auto createPillBtn = [this](const QString& text, bool isDefault) -> QPushButton* {
        auto* btn = new QPushButton(text, this);
        btn->setCheckable(true);
        btn->setChecked(isDefault);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 14px; padding: 6px 14px; font-size: 12px; font-weight: 600; }"
            "QPushButton:hover { background: %4; border-color: %5; }"
            "QPushButton:checked { background: %6; color: #ffffff; border-color: %6; }")
            .arg(Theme::BgSurface)
            .arg(Theme::TextSecondary)
            .arg(Theme::BorderSubtle)
            .arg(Theme::BgHover)
            .arg(Theme::BorderStrong)
            .arg(Theme::AccentCyan));
        return btn;
    };

    m_btnFilterAll = createPillBtn("All Applications", true);
    m_btnFilterUnused = createPillBtn("Potentially Unused", false);
    m_btnFilterLarge = createPillBtn("Large (>500MB)", false);
    m_btnFilterRecent = createPillBtn("Recently Installed", false);

    filterLayout->addWidget(m_btnFilterAll);
    filterLayout->addWidget(m_btnFilterUnused);
    filterLayout->addWidget(m_btnFilterLarge);
    filterLayout->addWidget(m_btnFilterRecent);

    // Connect radio-like pill behavior
    auto updatePills = [this](QPushButton* active) {
        m_btnFilterAll->setChecked(active == m_btnFilterAll);
        m_btnFilterUnused->setChecked(active == m_btnFilterUnused);
        m_btnFilterLarge->setChecked(active == m_btnFilterLarge);
        m_btnFilterRecent->setChecked(active == m_btnFilterRecent);
    };

    connect(m_btnFilterAll, &QPushButton::clicked, this, [this, updatePills]() {
        updatePills(m_btnFilterAll);
        onCategorySelected(-1);
    });
    connect(m_btnFilterUnused, &QPushButton::clicked, this, [this, updatePills]() {
        updatePills(m_btnFilterUnused);
        onCategorySelected(static_cast<int>(AppCategory::PotentiallyUnused));
    });
    connect(m_btnFilterLarge, &QPushButton::clicked, this, [this, updatePills]() {
        updatePills(m_btnFilterLarge);
        onCategorySelected(static_cast<int>(AppCategory::Large));
    });
    connect(m_btnFilterRecent, &QPushButton::clicked, this, [this, updatePills]() {
        updatePills(m_btnFilterRecent);
        onCategorySelected(static_cast<int>(AppCategory::RecentlyInstalled));
    });

    filterLayout->addStretch();

    // Search Box
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search installed applications...");
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setFixedWidth(240);
    connect(m_searchBox, &QLineEdit::textChanged, this, &ApplicationsPage::onSearchChanged);
    filterLayout->addWidget(m_searchBox);

    // Sort Combo
    auto* sortLbl = new QLabel("Sort:", this);
    sortLbl->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 600; text-transform: uppercase;").arg(Theme::TextSecondary));
    filterLayout->addWidget(sortLbl);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem("Name (A to Z)", static_cast<int>(InstalledAppManager::SortField::NameAsc));
    m_sortCombo->addItem("Name (Z to A)", static_cast<int>(InstalledAppManager::SortField::NameDesc));
    m_sortCombo->addItem("Size (Largest First)", static_cast<int>(InstalledAppManager::SortField::SizeDesc));
    m_sortCombo->addItem("Size (Smallest First)", static_cast<int>(InstalledAppManager::SortField::SizeAsc));
    m_sortCombo->addItem("Date (Newest First)", static_cast<int>(InstalledAppManager::SortField::DateDesc));
    m_sortCombo->addItem("Publisher (A to Z)", static_cast<int>(InstalledAppManager::SortField::PublisherAsc));
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ApplicationsPage::onSortChanged);
    filterLayout->addWidget(m_sortCombo);

    // Refresh button
    m_refreshBtn = new QPushButton("Rescan", this);
    m_refreshBtn->setProperty("class", "actionBtn");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ApplicationsPage::triggerScan);
    filterLayout->addWidget(m_refreshBtn);

    mainLayout->addLayout(filterLayout);

    // 4. Main Applications Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "Application Name", "Publisher", "Version", "Install Date", "Estimated Size", "Category"
    });

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(36);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &ApplicationsPage::onTableSelectionChanged);

    mainLayout->addWidget(m_table, 1);

    // 5. Details Inspector Panel (bottom)
    m_detailsPanel = new QFrame(this);
    m_detailsPanel->setStyleSheet(Theme::panelStyleSheet(8));
    auto* detailsLayout = new QVBoxLayout(m_detailsPanel);
    detailsLayout->setContentsMargins(16, 12, 16, 12);
    detailsLayout->setSpacing(8);

    auto* inspectHeader = new QHBoxLayout();
    auto* detailsTitle = new QLabel("Application Details & Safe Removal", m_detailsPanel);
    detailsTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
    inspectHeader->addWidget(detailsTitle);

    m_actionStatusLabel = new QLabel("", m_detailsPanel);
    m_actionStatusLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::AccentCyan));
    inspectHeader->addWidget(m_actionStatusLabel, 0, Qt::AlignRight);

    detailsLayout->addLayout(inspectHeader);

    auto* metaGrid = new QGridLayout();
    metaGrid->setSpacing(6);

    m_detailName = new QLabel("Select an application from the table above to inspect details.", m_detailsPanel);
    m_detailName->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    metaGrid->addWidget(m_detailName, 0, 0, 1, 3);

    m_detailPublisher = new QLabel("Publisher: --", m_detailsPanel);
    m_detailPublisher->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    metaGrid->addWidget(m_detailPublisher, 1, 0);

    m_detailVersion = new QLabel("Version: --", m_detailsPanel);
    m_detailVersion->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    metaGrid->addWidget(m_detailVersion, 1, 1);

    m_detailSize = new QLabel("Size: --", m_detailsPanel);
    m_detailSize->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    metaGrid->addWidget(m_detailSize, 1, 2);

    m_detailDate = new QLabel("Install Date: --", m_detailsPanel);
    m_detailDate->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    metaGrid->addWidget(m_detailDate, 2, 0);

    m_detailLocation = new QLabel("Location: --", m_detailsPanel);
    m_detailLocation->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    metaGrid->addWidget(m_detailLocation, 2, 1, 1, 2);

    detailsLayout->addLayout(metaGrid);

    // Evidence & Action row
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(12);

    m_detailEvidence = new QLabel("Evidence: No application selected.", m_detailsPanel);
    m_detailEvidence->setStyleSheet(QString("font-size: 12px; color: %1; background: %2; padding: 6px 10px; border-radius: 6px; border: 1px solid %3;")
                                        .arg(Theme::TextPrimary)
                                        .arg(Theme::BgSurfaceAlt)
                                        .arg(Theme::BorderSubtle));
    m_detailEvidence->setWordWrap(true);
    actionRow->addWidget(m_detailEvidence, 1);

    m_uninstallBtn = new QPushButton("Uninstall Application...", m_detailsPanel);
    m_uninstallBtn->setStyleSheet(
        QString("QPushButton { background-color: #be123c; color: #ffffff; font-weight: 600; font-size: 12px; padding: 7px 16px; border-radius: 6px; border: none; }"
                "QPushButton:hover { background-color: #e11d48; }"
                "QPushButton:disabled { background-color: %1; color: %2; }")
            .arg(Theme::BgSurfaceAlt)
            .arg(Theme::TextMuted));
    m_uninstallBtn->setEnabled(false);
    m_uninstallBtn->setCursor(Qt::PointingHandCursor);
    connect(m_uninstallBtn, &QPushButton::clicked, this, &ApplicationsPage::onUninstallClicked);
    actionRow->addWidget(m_uninstallBtn, 0);

    detailsLayout->addLayout(actionRow);

    mainLayout->addWidget(m_detailsPanel);
}

void ApplicationsPage::triggerScan() {
    if (m_isScanning) return;
    m_isScanning = true;

    m_statusPill->setText("Scanning Windows Registry...");
    m_statusPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentCyan));
    m_scanProgressBar->setVisible(true);
    m_refreshBtn->setEnabled(false);

    auto procs = m_cachedProcesses;
    auto manager = m_appManager;

    QThreadPool::globalInstance()->start([this, manager, procs]() {
        auto apps = manager->scanInstalledApplications(procs);
        QMetaObject::invokeMethod(this, [this, apps = std::move(apps)]() {
            onScanFinished(apps);
        }, Qt::QueuedConnection);
    });
}

void ApplicationsPage::onScanFinished(const std::vector<InstalledAppInfo>& apps) {
    m_allApps = apps;
    m_isScanning = false;
    m_scanProgressBar->setVisible(false);
    m_refreshBtn->setEnabled(true);

    m_statusPill->setText(QString("%1 applications found").arg(m_allApps.size()));
    m_statusPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentGreen));

    updateStatCards();
    applyFilterAndSort();

    // Calculate unused apps count for signal
    size_t unused = 0;
    for (const auto& app : m_allApps) {
        if (app.category == AppCategory::PotentiallyUnused) {
            unused++;
        }
    }
    emit scanCompleted(m_allApps.size(), unused);
}

void ApplicationsPage::updateStatCards() {
    m_totalCountVal->setText(QString::number(m_allApps.size()));

    uint64_t totalBytes = 0;
    size_t unusedCount = 0;
    size_t largeCount = 0;

    for (const auto& a : m_allApps) {
        totalBytes += a.estimatedSize;
        if (a.category == AppCategory::PotentiallyUnused) {
            unusedCount++;
        }
        if (a.category == AppCategory::Large) {
            largeCount++;
        }
    }

    m_totalSizeVal->setText(QString::fromStdString(InstalledAppManager::formatBytes(totalBytes)));
    m_unusedCountVal->setText(QString::number(unusedCount));
    m_largeCountVal->setText(QString::number(largeCount));
}

void ApplicationsPage::updateProcessContext(const std::vector<ProcessInfo>& processes) {
    m_cachedProcesses = processes;
}

void ApplicationsPage::onSearchChanged(const QString& query) {
    m_searchQuery = query;
    applyFilterAndSort();
}

void ApplicationsPage::onSortChanged(int index) {
    auto field = static_cast<InstalledAppManager::SortField>(m_sortCombo->itemData(index).toInt());
    m_activeSort = field;
    applyFilterAndSort();
}

void ApplicationsPage::onCategorySelected(int categoryIndex) {
    if (categoryIndex < 0) {
        m_activeCategory.reset();
    } else {
        m_activeCategory = static_cast<AppCategory>(categoryIndex);
    }
    applyFilterAndSort();
}

void ApplicationsPage::applyFilterAndSort() {
    std::vector<InstalledAppInfo> filtered = m_allApps;

    // Filter by Category
    if (m_activeCategory.has_value()) {
        filtered = InstalledAppManager::filterByCategory(filtered, m_activeCategory.value());
    }

    // Filter by Search Query
    if (!m_searchQuery.trimmed().isEmpty()) {
        filtered = InstalledAppManager::filterByName(filtered, m_searchQuery.toStdString());
    }

    // Sort
    InstalledAppManager::sortApplications(filtered, m_activeSort);

    m_displayedApps = std::move(filtered);
    populateTable();
}

void ApplicationsPage::populateTable() {
    m_table->setRowCount(0);
    m_table->setRowCount(static_cast<int>(m_displayedApps.size()));

    for (int row = 0; row < static_cast<int>(m_displayedApps.size()); ++row) {
        const auto& app = m_displayedApps[row];

        auto* itemTitle = new QTableWidgetItem(QString::fromStdString(app.displayName));
        itemTitle->setData(Qt::UserRole, row);
        m_table->setItem(row, 0, itemTitle);

        auto* itemPub = new QTableWidgetItem(app.publisher.empty() ? "--" : QString::fromStdString(app.publisher));
        m_table->setItem(row, 1, itemPub);

        auto* itemVer = new QTableWidgetItem(app.displayVersion.empty() ? "--" : QString::fromStdString(app.displayVersion));
        m_table->setItem(row, 2, itemVer);

        auto* itemDate = new QTableWidgetItem(app.installDate.empty() ? "--" : QString::fromStdString(app.installDate));
        m_table->setItem(row, 3, itemDate);

        auto* itemSize = new QTableWidgetItem(QString::fromStdString(app.formattedSize()));
        m_table->setItem(row, 4, itemSize);

        // Category Badge
        auto* itemCat = new QTableWidgetItem(QString::fromStdString(appCategoryToString(app.category)));
        if (app.category == AppCategory::PotentiallyUnused) {
            itemCat->setForeground(Theme::colorAccentAmber());
        } else if (app.category == AppCategory::Large) {
            itemCat->setForeground(Theme::colorAccentPurple());
        } else if (app.category == AppCategory::RecentlyInstalled) {
            itemCat->setForeground(Theme::colorAccentCyan());
        } else {
            itemCat->setForeground(Theme::colorTextSecondary());
        }
        m_table->setItem(row, 5, itemCat);
    }

    if (m_displayedApps.empty()) {
        updateDetailsPanel(nullptr);
    } else {
        m_table->selectRow(0);
    }
}

void ApplicationsPage::onTableSelectionChanged() {
    auto selectedItems = m_table->selectedItems();
    if (selectedItems.empty()) {
        updateDetailsPanel(nullptr);
        return;
    }

    int row = selectedItems.first()->row();
    if (row >= 0 && row < static_cast<int>(m_displayedApps.size())) {
        m_selectedRow = row;
        updateDetailsPanel(&m_displayedApps[row]);
    }
}

void ApplicationsPage::updateDetailsPanel(const InstalledAppInfo* app) {
    if (!app) {
        m_detailName->setText("Select an application from the table above to inspect details.");
        m_detailPublisher->setText("Publisher: --");
        m_detailVersion->setText("Version: --");
        m_detailSize->setText("Size: --");
        m_detailDate->setText("Install Date: --");
        m_detailLocation->setText("Location: --");
        m_detailEvidence->setText("Evidence: No application selected.");
        m_uninstallBtn->setEnabled(false);
        m_actionStatusLabel->setText("");
        return;
    }

    m_detailName->setText(QString::fromStdString(app->displayName));
    m_detailPublisher->setText(QString("Publisher: %1").arg(app->publisher.empty() ? "Unknown" : QString::fromStdString(app->publisher)));
    m_detailVersion->setText(QString("Version: %1").arg(app->displayVersion.empty() ? "N/A" : QString::fromStdString(app->displayVersion)));
    m_detailSize->setText(QString("Size: %1").arg(QString::fromStdString(app->formattedSize())));
    m_detailDate->setText(QString("Install Date: %1").arg(app->installDate.empty() ? "Not recorded" : QString::fromStdString(app->installDate)));
    m_detailLocation->setText(QString("Location: %1").arg(app->installLocation.empty() ? "Registry registered" : QString::fromStdString(app->installLocation)));

    QString evidenceText = QString("Usage Evidence: %1").arg(app->usageEvidence.empty() ? "No specific usage evidence recorded." : QString::fromStdString(app->usageEvidence));
    m_detailEvidence->setText(evidenceText);

    bool canUninstall = app->canUninstall && !app->uninstallString.empty() && !app->isSystemComponent;
    m_uninstallBtn->setEnabled(canUninstall);

    if (!canUninstall) {
        m_uninstallBtn->setToolTip("System component or uninstaller not registered by vendor.");
    } else {
        m_uninstallBtn->setToolTip("Launch official registered Windows uninstaller.");
    }
}

void ApplicationsPage::onUninstallClicked() {
    if (m_selectedRow < 0 || m_selectedRow >= static_cast<int>(m_displayedApps.size())) {
        return;
    }

    const auto& app = m_displayedApps[m_selectedRow];

    UninstallConfirmDialog dialog(app, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_actionStatusLabel->setText(QString("Launching vendor uninstaller for %1...").arg(QString::fromStdString(app.displayName)));

    auto manager = m_appManager;
    auto appCopy = app;

    QThreadPool::globalInstance()->start([this, manager, appCopy]() {
        auto result = manager->uninstallApplication(appCopy, false);
        QMetaObject::invokeMethod(this, [this, result, appCopy]() {
            if (result.success) {
                m_actionStatusLabel->setText(QString("Uninstaller started. Complete the process in the vendor setup window."));
                QMessageBox::information(this, "Uninstaller Launched",
                    QString("The registered uninstaller for \"%1\" has been launched.\n\n"
                            "Please complete the removal in the vendor's wizard, then click 'Rescan' to update the application list.")
                        .arg(QString::fromStdString(appCopy.displayName)));
            } else {
                m_actionStatusLabel->setText(QString("Uninstall could not proceed: %1").arg(QString::fromStdString(result.message)));
                QMessageBox::warning(this, "Uninstaller Notification",
                    QString("Could not launch uninstaller:\n\n%1").arg(QString::fromStdString(result.message)));
            }
        }, Qt::QueuedConnection);
    });
}

} // namespace pulse::ui
