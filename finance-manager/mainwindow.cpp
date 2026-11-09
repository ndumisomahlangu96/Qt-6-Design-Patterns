#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the central widget programmatically (No UI file needed)
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // Set the window title explicitly in C++
    setWindowTitle("Finance Manager");

    setupModel();
    setupFlexibleUI();
    setupMenuBar(); // Initialize the menu bar
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupMenuBar()
{
    // Fetch the standard application style to access built-in icons
    QStyle *style = QApplication::style();

    // --- File Menu ---
    QMenu *fileMenu = menuBar()->addMenu("File");

    QAction *newAction = new QAction(style->standardIcon(QStyle::SP_FileIcon), "New", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("Clear current records and start fresh");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewData);
    fileMenu->addAction(newAction);

    // --- Open File Action ---
    QAction *openAction = new QAction(style->standardIcon(QStyle::SP_DirOpenIcon), "Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an existing expense CSV file");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);
    // ------------------------

    QAction *saveAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save current expense records to disk");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);
    fileMenu->addAction(saveAction);

    // ---  Save As Action ---
    QAction *saveAsAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip("Save current expense records to a new file location");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveDataAs);
    fileMenu->addAction(saveAsAction);
    // --------------------------

    QAction *exportAction = new QAction(style->standardIcon(QStyle::SP_ArrowDown), "Export to CSV...", this);
    exportAction->setStatusTip("Export all records to a CSV file");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCSV);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);

    // --- Edit Menu ---
    QMenu *editMenu = menuBar()->addMenu("Edit");

    QAction *deleteAction = new QAction(style->standardIcon(QStyle::SP_TrashIcon), "Delete Selected Record", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Delete the currently selected record from the ledger");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    editMenu->addAction(deleteAction);

    // --- Sort Action ---
    QAction *sortAction = new QAction(style->standardIcon(QStyle::SP_BrowserReload), "Sort Ledger...", this);
    sortAction->setStatusTip("Sort the ledger by a specific variable");
    connect(sortAction, &QAction::triggered, this, &MainWindow::onSortLedger);
    editMenu->addAction(sortAction);

    // --- Find and Replace Action ---
    QAction *findAction = new QAction(QIcon::fromTheme("edit-find-replace", style->standardIcon(QStyle::SP_FileDialogContentsView)), "Find and Replace...", this);
    findAction->setShortcut(QKeySequence::Replace);
    findAction->setStatusTip("Search and replace text across the entire database");
    connect(findAction, &QAction::triggered, this, &MainWindow::onFindAndReplace);
    editMenu->addAction(findAction);

    // --- Help Menu ---
    QMenu *helpMenu = menuBar()->addMenu("Help");

    QAction *aboutAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxInformation), "About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);

    // --- Main Toolbar ---
    QToolBar *mainToolBar = new QToolBar("Main Toolbar", this);
    mainToolBar->setMovable(false);
    this->addToolBar(Qt::TopToolBarArea, mainToolBar);

    mainToolBar->addAction(newAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();

    QAction *totalSpendingAction = new QAction(style->standardIcon(QStyle::SP_DialogApplyButton), "Total Spending", this);
    totalSpendingAction->setStatusTip("Calculate the total of all expenses in the ledger");
    connect(totalSpendingAction, &QAction::triggered, this, &MainWindow::onCalculateTotalSpending);

    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->addAction(totalSpendingAction);

    // --- Ledger Statistics Action ---
    QAction *statsAction = new QAction(style->standardIcon(QStyle::SP_FileDialogDetailedView), "Ledger Statistics", this);
    statsAction->setStatusTip("Generate statistical summary of the current ledger data");
    connect(statsAction, &QAction::triggered, this, &MainWindow::onGenerateStats);
    mainToolBar->addAction(statsAction);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    event->accept();
}

QString MainWindow::getDataFilePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path + "/expenses.csv";
}

void MainWindow::saveData()
{
    QString savePath = currentFilePath.isEmpty() ? getDataFilePath() : currentFilePath;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Date,Project/Site,Category,Description,Amount\n";

        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QString text = expenseModel->item(row, col)->text();
                text.replace("\"", "\"\"");
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        currentFilePath = savePath;
        statusBar()->showMessage("Data saved successfully to: " + currentFilePath, 3000);
    }
    else
    {
        statusBar()->showMessage("Failed to save data.", 3000);
    }
}

void MainWindow::saveDataAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", QDir::homePath(), "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Date,Project/Site,Category,Description,Amount\n";

        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QString text = expenseModel->item(row, col)->text();
                text.replace("\"", "\"\"");
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        currentFilePath = fileName;
        statusBar()->showMessage("File saved successfully to new location.", 3000);
    } else {
        QMessageBox::warning(this, "Save As Error", "Could not write to the selected file.");
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Expense File", QDir::homePath(), "All Files (*);;CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error", "Could not open the selected file.");
        return;
    }

    currentFilePath = fileName;
    expenseModel->removeRows(0, expenseModel->rowCount());

    QTextStream in(&file);
    bool isFirstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        QStringList fields;
        QString currentField;
        bool inQuotes = false;

        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '\"') {
                if (i + 1 < line.length() && line[i+1] == '\"') {
                    currentField.append('\"');
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField);
                currentField.clear();
            } else {
                currentField.append(c);
            }
        }
        fields.append(currentField);

        if (fields.size() >= 5) {
            QList<QStandardItem *> newRow;
            for (int col = 0; col < 5; ++col) {
                QStandardItem *item = new QStandardItem(fields[col]);
                if (col == 4) {
                    item->setData(fields[col].toDouble(), Qt::UserRole);
                } else {
                    item->setData(fields[col], Qt::UserRole);
                }
                newRow << item;
            }
            expenseModel->appendRow(newRow);
        }
    }
    file.close();

    updateChart();
    statusBar()->showMessage("File opened successfully.", 3000);
}

void MainWindow::setupModel()
{
    expenseModel = new QStandardItemModel(0, 5, this);
    expenseModel->setHorizontalHeaderLabels({"Date", "Project/Site", "Category", "Description", "Amount"});
    expenseModel->setSortRole(Qt::UserRole);
    connect(expenseModel, &QStandardItemModel::itemChanged, this, &MainWindow::updateChart);
}

void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    QTabWidget *tabWidget = new QTabWidget(this);

    // --- TAB 1: Expense Entry ---
    QWidget *entryTab = new QWidget();
    QVBoxLayout *entryLayout = new QVBoxLayout(entryTab);
    QGroupBox *formGroup = new QGroupBox("New Expense Record");
    QFormLayout *formLayout = new QFormLayout(formGroup);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);

    projectCombo = new QComboBox();
    projectCombo->setEditable(true);
    projectCombo->addItems({"Rental Property", "Residential Property", "General Overhead"});

    categoryCombo = new QComboBox();
    categoryCombo->setEditable(true);
    categoryCombo->addItems({"Raw Materials", "Maintenance", "Equipment", "Fuel","Human Labour"});

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");

    descriptionEdit = new QLineEdit();

    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Project:", projectCombo);
    formLayout->addRow("Expense Category:", categoryCombo);
    formLayout->addRow("Description:", descriptionEdit);
    formLayout->addRow("Amount:", amountSpin);

    QPushButton *saveBtn = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton), "Record Expense");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onAddExpenseClicked);

    entryLayout->addWidget(formGroup);
    entryLayout->addWidget(saveBtn, 0, Qt::AlignRight);
    entryLayout->addStretch();

    // --- TAB 2: Ledger View ---
    QWidget *ledgerTab = new QWidget();
    QVBoxLayout *ledgerLayout = new QVBoxLayout(ledgerTab);

    ledgerView = new QTableView();
    ledgerView->setModel(expenseModel);
    ledgerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ledgerView->setAlternatingRowColors(true);
    ledgerLayout->addWidget(ledgerView);

    // --- TAB 3: Visual Dashboard ---
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardTab);

    categorySeries = new QPieSeries();
    connect(categorySeries, &QPieSeries::clicked, this, &MainWindow::onPieSliceClicked);

    expenseChart = new QChart();
    expenseChart->addSeries(categorySeries);
    expenseChart->setTitle("Expenses by Category");
    expenseChart->setAnimationOptions(QChart::SeriesAnimations);
    expenseChart->legend()->setAlignment(Qt::AlignRight);

    chartView = new QChartView(expenseChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    dashboardLayout->addWidget(chartView);

    // Add tabs
    tabWidget->addTab(entryTab, "Data Entry");
    tabWidget->addTab(ledgerTab, "Ledger Reports");
    tabWidget->addTab(dashboardTab, "Visual Dashboard");

    mainLayout->addWidget(tabWidget);
}

void MainWindow::onAddExpenseClicked()
{
    QList<QStandardItem *> newRow;

    QStandardItem *dateItem = new QStandardItem(dateEdit->date().toString("yyyy-MM-dd"));
    dateItem->setData(dateItem->text(), Qt::UserRole);
    newRow << dateItem;

    QStandardItem *projectItem = new QStandardItem(projectCombo->currentText());
    projectItem->setData(projectItem->text(), Qt::UserRole);
    newRow << projectItem;

    QStandardItem *categoryItem = new QStandardItem(categoryCombo->currentText());
    categoryItem->setData(categoryItem->text(), Qt::UserRole);
    newRow << categoryItem;

    QStandardItem *descItem = new QStandardItem(descriptionEdit->text());
    descItem->setData(descItem->text(), Qt::UserRole);
    newRow << descItem;

    QStandardItem *amountItem = new QStandardItem(QString::number(amountSpin->value(), 'f', 2));
    amountItem->setData(amountSpin->value(), Qt::UserRole);
    newRow << amountItem;

    expenseModel->appendRow(newRow);

    descriptionEdit->clear();
    amountSpin->setValue(0.0);

    updateChart();
}

void MainWindow::updateChart()
{
    categorySeries->clear();
    QMap<QString, double> categoryTotals;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        QString category = expenseModel->item(row, 2)->text();
        double amount = expenseModel->item(row, 4)->text().toDouble();
        categoryTotals[category] += amount;
    }

    for (auto it = categoryTotals.begin(); it != categoryTotals.end(); ++it) {
        QPieSlice *slice = categorySeries->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }
}

void MainWindow::onNewData()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Data", "Are you sure you want to clear all expense records?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        expenseModel->removeRows(0, expenseModel->rowCount());
        updateChart();
        statusBar()->showMessage("All records cleared.", 3000);
    }
}

void MainWindow::onExportCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Expenses", QDir::homePath(), "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Date,Project/Site,Category,Description,Amount\n";

        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                rowData << "\"" + expenseModel->item(row, col)->text() + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        statusBar()->showMessage("Data exported to CSV successfully.", 3000);
    } else {
        QMessageBox::warning(this, "Export Error", "Could not write to the selected file.");
    }
}

void MainWindow::onExit()
{
    this->close();
}

void MainWindow::onDeleteSelected()
{
    QModelIndexList selectedRows = ledgerView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a row in the Ledger Reports tab to delete.");
        return;
    }

    for (int i = selectedRows.count() - 1; i >= 0; i--) {
        expenseModel->removeRow(selectedRows.at(i).row());
    }

    updateChart();
    statusBar()->showMessage("Record deleted.", 3000);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Finance Manager",
                       "<b>Finance Manager</b><br><br>"
                       "A simple, Qt-based application for tracking project expenses, "
                       "viewing ledgers, and analyzing financial data.");
}

void MainWindow::onSortLedger()
{
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok;

    QString item = QInputDialog::getItem(this, "Sort Ledger",
                                         "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        int columnIndex = columns.indexOf(item);
        if (columnIndex != -1) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Sort Order",
                                          QString("Sort the '%1' column in Ascending order?\n(Select 'No' for Descending)").arg(item),
                                          QMessageBox::Yes | QMessageBox::No);

            Qt::SortOrder order = (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder;
            expenseModel->sort(columnIndex, order);

            statusBar()->showMessage(QString("Ledger sorted by %1.").arg(item), 3000);
        }
    }
}

void MainWindow::onFindAndReplace()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Find and Replace");
    dialog.setMinimumWidth(300);

    QFormLayout form(&dialog);

    QLineEdit *findInput = new QLineEdit(&dialog);
    findInput->setPlaceholderText("Enter target string...");

    QLineEdit *replaceInput = new QLineEdit(&dialog);
    replaceInput->setPlaceholderText("Enter replacement string...");

    form.addRow("Find:", findInput);
    form.addRow("Replace with:", replaceInput);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    buttonBox.button(QDialogButtonBox::Ok)->setText("Replace All");
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString targetText = findInput->text();
        QString replaceText = replaceInput->text();

        if (targetText.isEmpty()) {
            return;
        }

        int replacedCount = 0;

        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QStandardItem *item = expenseModel->item(row, col);
                QString currentText = item->text();

                if (currentText.contains(targetText, Qt::CaseInsensitive)) {
                    currentText.replace(targetText, replaceText, Qt::CaseInsensitive);
                    item->setText(currentText);

                    if (col == 4) {
                        item->setData(currentText.toDouble(), Qt::UserRole);
                    } else {
                        item->setData(currentText, Qt::UserRole);
                    }

                    replacedCount++;
                }
            }
        }

        if (replacedCount > 0) {
            statusBar()->showMessage(QString("Successfully replaced %1 occurrence(s).").arg(replacedCount), 5000);

            QTabWidget *tabWidget = this->findChild<QTabWidget*>();
            if (tabWidget) {
                tabWidget->setCurrentIndex(1);
            }
        } else {
            QMessageBox::information(this, "Not Found", "Could not find '" + targetText + "' in the database.");
        }
    }
}

void MainWindow::onPieSliceClicked(QPieSlice *slice)
{
    QString category = slice->label().section(':', 0, 0).trimmed();
    QMap<QString, double> projectTotals;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        QString rowCategory = expenseModel->item(row, 2)->text();
        if (rowCategory == category) {
            QString project = expenseModel->item(row, 1)->text();
            double amount = expenseModel->item(row, 4)->text().toDouble();
            projectTotals[project] += amount;
        }
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(category + " - Project Breakdown");
    dialog->resize(500, 400);

    QPieSeries *projectSeries = new QPieSeries();

    for (auto it = projectTotals.begin(); it != projectTotals.end(); ++it) {
        QPieSlice *pSlice = projectSeries->append(it.key(), it.value());
        pSlice->setLabelVisible(true);
        pSlice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }

    QChart *projectChart = new QChart();
    projectChart->addSeries(projectSeries);
    projectChart->setTitle(QString("Budget Distribution for '%1'").arg(category));
    projectChart->setAnimationOptions(QChart::SeriesAnimations);
    projectChart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *pChartView = new QChartView(projectChart);
    pChartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(pChartView);

    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dialog->exec();
    delete dialog;
}

void MainWindow::onCalculateTotalSpending()
{
    double total = 0.0;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        double amount = expenseModel->item(row, 4)->data(Qt::UserRole).toDouble();
        total += amount;
    }

    QString formattedTotal = QString::number(total, 'f', 2);
    statusBar()->showMessage("Total Spending: R " + formattedTotal, 5000);

    QMessageBox::information(this, "Total Spending",
                             QString("The total spending for all records is:\n\nR %1").arg(formattedTotal));
}

void MainWindow::onGenerateStats()
{
    int rowCount = expenseModel->rowCount();
    if (rowCount == 0) {
        QMessageBox::information(this, "Ledger Statistics", "The ledger is currently empty. Add data to view statistics.");
        return;
    }

    double minExpense = expenseModel->item(0, 4)->data(Qt::UserRole).toDouble();
    double maxExpense = minExpense;
    double totalExpense = 0.0;

    QMap<QString, int> categoryCounts;
    QMap<QString, int> projectCounts;

    for (int row = 0; row < rowCount; ++row) {
        double amount = expenseModel->item(row, 4)->data(Qt::UserRole).toDouble();
        QString project = expenseModel->item(row, 1)->text();
        QString category = expenseModel->item(row, 2)->text();

        if (amount < minExpense) minExpense = amount;
        if (amount > maxExpense) maxExpense = amount;
        totalExpense += amount;

        categoryCounts[category]++;
        projectCounts[project]++;
    }

    double avgExpense = totalExpense / rowCount;

    QString topCategory;
    int maxCatCount = 0;
    for (auto it = categoryCounts.begin(); it != categoryCounts.end(); ++it) {
        if (it.value() > maxCatCount) {
            maxCatCount = it.value();
            topCategory = it.key();
        }
    }

    QString topProject;
    int maxProjCount = 0;
    for (auto it = projectCounts.begin(); it != projectCounts.end(); ++it) {
        if (it.value() > maxProjCount) {
            maxProjCount = it.value();
            topProject = it.key();
        }
    }

    QString statsMessage = QString(
                               "<h3>Ledger Statistical Summary</h3>"
                               "<b>Total Records Analyzed:</b> %1<br><br>"
                               "<b>Average Expense:</b> R %2<br>"
                               "<b>Highest Single Expense:</b> R %3<br>"
                               "<b>Lowest Single Expense:</b> R %4<br><br>"
                               "<b>Most Frequent Category:</b> %5 (%6 entries)<br>"
                               "<b>Most Frequent Project:</b> %7 (%8 entries)"
                               ).arg(rowCount)
                               .arg(avgExpense, 0, 'f', 2)
                               .arg(maxExpense, 0, 'f', 2)
                               .arg(minExpense, 0, 'f', 2)
                               .arg(topCategory).arg(maxCatCount)
                               .arg(topProject).arg(maxProjCount);

    QMessageBox::information(this, "Ledger Statistics", statsMessage);
}