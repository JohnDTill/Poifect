#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "hashwrite.h"

static MainWindow* main_window;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){
    ui->setupUi(this);
    main_window = this;

    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(writeHashFunction()));
    connect(ui->inputEdit, SIGNAL(textChanged()), this, SLOT(parseInputField()));
    connect(ui->sparsityEdit, SIGNAL(editingFinished()), this, SLOT(parseInputField()));
    connect(ui->delimiterEdit, SIGNAL(editingFinished()), this, SLOT(parseInputField()));

    parseInputField();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::parseInputField(){
    QString delimiter_str = ui->delimiterEdit->text();
    if(delimiter_str.size() > 1){
        ui->outputEdit->setText("delimiter must be single char");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText("🤔   Input error");
        return;
    }else if(delimiter_str.isEmpty()){
        ui->outputEdit->setText("delimiter required");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText("🤔   Input error");
        return;
    }
    QChar delimiter = delimiter_str.front();

    QString source = ui->inputEdit->toPlainText();
    QVector<QStringRef> rows = source.splitRef('\n');
    if(rows.size() > 1 && rows.back().isEmpty()) rows.pop_back();

    keys.resize(static_cast<std::vector<std::string>::size_type>(rows.size()));
    vals.resize(static_cast<std::vector<std::string>::size_type>(rows.size()));

    for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
        QVector<QStringRef> entries = rows[static_cast<QVector<QStringRef>::size_type>(i)].split(delimiter);
        if(entries.size() < 2){
            ui->outputEdit->setText("Row " + QString::number(i+1) + " does not have 2 entries");
            ui->pushButton->setEnabled(false);
            ui->statusLabel->setText("🤔   Input error");
            return;
        }

        std::string key = entries[0].toString().toStdString();
        for(std::vector<std::string>::size_type j = 0; j < i; j++){
            if(keys[j] == key){
                ui->outputEdit->setText("Key on row " + QString::number(i+1) +
                                        " duplicates key on row " + QString::number(j+1));
                ui->pushButton->setEnabled(false);
                ui->statusLabel->setText("🤔   Input error");
                return;
            }
        }

        keys[i] = key;
        vals[i] = entries[1].toString().toStdString();
    }

    ui->countEdit->setText(QString::number(keys.size()));

    bool success;
    uint target_size = ui->sparsityEdit->text().toUInt(&success);

    if(!success){
        empty_space = -1;
        ui->outputEdit->setText("acceptable dense size is not a valid whole number");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText("🤔   Input error");
        return;
    }

    empty_space = target_size - keys.size();
    if(empty_space < 0){
        ui->outputEdit->setText("acceptable dense size " + QString::number(target_size) +
                                " is less than number of keys " + QString::number(keys.size()));
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText("🤔   Input error");
        return;
    }

    ui->pushButton->setEnabled(true);
    ui->outputEdit->clear();
    ui->statusLabel->setText("🧐   Ready to search");
}

static uint8_t progress = 0;
static bool terminated = true;
static std::string msg;

static void callback(){
    main_window->ui->progressBar->setValue(progress);
    main_window->ui->statusLabel->setText(QString::fromStdString(msg));
    QCoreApplication::processEvents();
}

void MainWindow::writeHashFunction(){
    if(terminated==false){
        terminated = true;
        ui->progressBar->setValue(0);
        return;
    }

    terminated = false;
    QString old_btn_msg = ui->pushButton->text();
    ui->pushButton->setText("Terminate");
    ui->inputEdit->setEnabled(false);
    ui->returnEdit->setEnabled(false);
    ui->defaultEdit->setEnabled(false);
    ui->delimiterEdit->setEnabled(false);
    ui->sparsityEdit->setEnabled(false);

    std::string r_type = ui->returnEdit->text().toStdString();
    std::string dflt = ui->defaultEdit->text().toStdString();

    SearchResult result = writePerfectHash<&callback>(keys, vals, empty_space, r_type, dflt, progress, terminated, msg);

    ui->outputEdit->setText(QString::fromStdString(result.hash_source));
    ui->pushButton->setText(old_btn_msg);
    ui->inputEdit->setEnabled(true);
    ui->returnEdit->setEnabled(true);
    ui->defaultEdit->setEnabled(true);
    ui->delimiterEdit->setEnabled(true);
    ui->sparsityEdit->setEnabled(true);
    terminated = true;

    callback();
}
