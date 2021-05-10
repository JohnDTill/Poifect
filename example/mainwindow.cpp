#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../hashsearch.h"
#include "../hashsearch2.h"

static MainWindow* main_window;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){
    ui->setupUi(this);
    main_window = this;

    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(writeHashFunction()));
    connect(ui->inputEdit, SIGNAL(textChanged()), this, SLOT(parseInputField()));
    connect(ui->expansionEdit, SIGNAL(editingFinished()), this, SLOT(parseInputField()));
    connect(ui->reductionEdit, SIGNAL(editingFinished()), this, SLOT(parseInputField()));
    connect(ui->layerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(parseInputField()));
    connect(ui->intCheckBox, SIGNAL(stateChanged(int)), this, SLOT(parseInputField()));
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
        ui->statusLabel->setText(":\\   Input error");
        return;
    }else if(delimiter_str.isEmpty()){
        ui->outputEdit->setText("delimiter required");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText(":|   Input error");
        return;
    }
    QChar delimiter = delimiter_str.front();

    QString source = ui->inputEdit->toPlainText();
    QVector<QStringRef> rows = source.splitRef('\n');
    if(rows.size() > 1 && rows.back().isEmpty()) rows.pop_back();

    if(rows.size() == 1){
        ui->pushButton->setEnabled(false);
        ui->outputEdit->setText("require at least 2 entries");
        ui->statusLabel->setText(":(   Input error");
        return;
    }

    keys.clear();
    vals.clear();

    for(int i = 0; i < rows.size(); i++){
        QVector<QStringRef> entries = rows[static_cast<QVector<QStringRef>::size_type>(i)].split(delimiter);
        if(entries.size() < 2){
            ui->outputEdit->setText("Row " + QString::number(i+1) + " does not have 2 entries");
            ui->pushButton->setEnabled(false);
            ui->statusLabel->setText(":[   Input error");
            return;
        }

        std::string key = entries[0].toString().toStdString();
        for(int j = 0; j < i; j++){
            if(keys[j] == key){
                ui->outputEdit->setText("Key on row " + QString::number(i+1) +
                                        " duplicates key on row " + QString::number(j+1));
                ui->pushButton->setEnabled(false);
                ui->statusLabel->setText("(╯°□°）╯︵ ┻━┻   Input error");
                return;
            }
        }

        keys.push_back(key);
        vals.push_back(entries[1].toString().toStdString());
    }

    bool success;
    expansion = ui->expansionEdit->text().toUInt(&success);

    if(!success || expansion==0){
        empty_space = -1;
        ui->outputEdit->setText("expansion must be a whole number");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText(">:(   Input error");
        return;
    }

    reduction = ui->reductionEdit->text().toUInt(&success);

    if(!success || reduction==0){
        empty_space = -1;
        ui->outputEdit->setText("reduction must be a whole number");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText(":|   Input error");
        return;
    }

    if(!ui->layerCheckBox->isChecked() && reduction > expansion){
        empty_space = -1;
        ui->outputEdit->setText("single-layer map requires reduction <= expansion");
        ui->pushButton->setEnabled(false);
        ui->statusLabel->setText(":/   Input error");
        return;
    }

    if(ui->intCheckBox->isChecked()){
        number_keys.clear();

        for(const std::string& key : keys){
            number_keys.push_back(QString::fromStdString(key).toULong(&success));
            if(!success){
                empty_space = -1;
                ui->outputEdit->setText("keys are not valid positive integers");
                ui->pushButton->setEnabled(false);
                ui->statusLabel->setText(":O    Input error");
                return;
            }
        }
    }

    ui->pushButton->setEnabled(true);
    ui->outputEdit->clear();
    ui->statusLabel->setText(":)    Ready to search");
}

void MainWindow::writeHashFunction(){
    QString old_btn_msg = ui->pushButton->text();
    ui->pushButton->setText("Terminate");
    ui->inputEdit->setEnabled(false);
    ui->expansionEdit->setEnabled(false);
    ui->reductionEdit->setEnabled(false);
    ui->intCheckBox->setEnabled(false);
    ui->layerCheckBox->setEnabled(false);
    ui->defaultEdit->setEnabled(false);
    ui->delimiterEdit->setEnabled(false);
    ui->nameEdit->setEnabled(false);

    std::string dflt = ui->defaultEdit->text().toStdString();
    std::string name = ui->nameEdit->text().toStdString();
    bool nonKeyLookup = !ui->keyOnlyCheckBox->isChecked();
    std::string hash_str;
    bool success;

    if(ui->intCheckBox->isChecked()){
        if(ui->layerCheckBox->isChecked())
            success = hashSearch2<uint32_t>(number_keys, vals, hash_str, name, dflt, expansion, reduction, nonKeyLookup);
        else
            success = hashSearch<uint32_t>(number_keys, vals, hash_str, name, dflt, expansion, reduction, nonKeyLookup);
    }else{
        if(ui->layerCheckBox->isChecked())
            success = hashSearch2<std::string>(keys, vals, hash_str, name, dflt, expansion, reduction, nonKeyLookup);
        else
            success = hashSearch<std::string>(keys, vals, hash_str, name, dflt, expansion, reduction, nonKeyLookup);
    }

    if(success){
        ui->outputEdit->setText(QString::fromStdString(hash_str));
        ui->statusLabel->setText(":D   Victory!");
    }else{
        ui->outputEdit->setText("Search Failed");
        ui->statusLabel->setText(":'(   No suitable hash function found");
    }

    ui->pushButton->setText(old_btn_msg);
    ui->inputEdit->setEnabled(true);
    ui->expansionEdit->setEnabled(true);
    ui->reductionEdit->setEnabled(true);
    ui->intCheckBox->setEnabled(true);
    ui->layerCheckBox->setEnabled(true);
    ui->defaultEdit->setEnabled(true);
    ui->delimiterEdit->setEnabled(true);
    ui->nameEdit->setEnabled(true);
}
