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
}

MainWindow::~MainWindow(){
    delete ui;
}

static uint8_t progress = 0;
static bool terminated = true;
static std::string msg;

#ifdef __EMSCRIPTEN__ //WASM code isn't updating correctly
#include <emscripten/emscripten.h>
#endif

static void callback(){
    main_window->ui->progressBar->setValue(progress);
    main_window->ui->statusLabel->setText(QString::fromStdString(msg));
    QCoreApplication::processEvents();
    #ifdef __EMSCRIPTEN__ //WASM code isn't updating correctly
    emscripten_sleep(1);
    #endif
}

void MainWindow::writeHashFunction(){
    if(terminated==false){
        terminated = true;
        ui->progressBar->setValue(0);
        return;
    }

    QString delimiter_str = ui->delimiterEdit->text();
    if(delimiter_str.size() > 1){
        ui->outputEdit->setText("delimiter must be single char");
        return;
    }else if(delimiter_str.isEmpty()){
        ui->outputEdit->setText("delimiter required");
        return;
    }
    QChar delimiter = delimiter_str.front();

    #ifndef __EMSCRIPTEN__ //WASM code isn't splitting correctly
    QVector<QStringRef> rows = ui->inputEdit->toPlainText().splitRef('\n');
    if(rows.size() > 1 && rows.back().isEmpty()) rows.pop_back();

    std::vector<std::string> keys(static_cast<std::vector<std::string>::size_type>(rows.size()));
    std::vector<std::string> vals(static_cast<std::vector<std::string>::size_type>(rows.size()));

    for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){        
        QVector<QStringRef> entries = rows[static_cast<QVector<QStringRef>::size_type>(i)].split(delimiter);
        if(entries.size() < 2){
            if(entries.size()==0){
                ui->outputEdit->setText("Row " + QString::number(i+1) + " NO_ENTRY, rows " + QString::number(rows.size()));
                return;
            }

            ui->outputEdit->setText("Row " + QString::number(i+1) + " does not have 2 entries, rows " + QString::number(rows.size())
                                    + "\n" + entries.front());
            return;
        }

        std::string key = entries[0].toString().toStdString();
        for(std::vector<std::string>::size_type j = 0; j < i; j++){
            if(keys[j] == key){
                ui->outputEdit->setText("Key on row " + QString::number(i+1) +
                                        " duplicates key on row " + QString::number(j+1));
                return;
            }
        }

        keys[i] = key;
        vals[i] = entries[1].toString().toStdString();
    }
    #else
    auto rows = ui->inputEdit->toPlainText().split('\n');
    if(rows.size() > 1 && rows.back().isEmpty()) rows.pop_back();

    std::vector<std::string> keys(static_cast<std::vector<std::string>::size_type>(rows.size()));
    std::vector<std::string> vals(static_cast<std::vector<std::string>::size_type>(rows.size()));

    for(std::vector<std::string>::size_type i = 0; i < keys.size(); i++){
        auto entries = rows[static_cast<QVector<QStringRef>::size_type>(i)].split(delimiter);
        if(entries.size() < 2){
            if(entries.size()==0){
                ui->outputEdit->setText("Row " + QString::number(i+1) + " NO_ENTRY, rows " + QString::number(rows.size()));
                return;
            }

            ui->outputEdit->setText("Row " + QString::number(i+1) + " does not have 2 entries, rows " + QString::number(rows.size())
                                    + "\n" + entries.front());
            return;
        }

        std::string key = entries[0].toStdString();
        for(std::vector<std::string>::size_type j = 0; j < i; j++){
            if(keys[j] == key){
                ui->outputEdit->setText("Key on row " + QString::number(i+1) +
                                        " duplicates key on row " + QString::number(j+1));
                return;
            }
        }

        keys[i] = key;
        vals[i] = entries[1].toStdString();
    }
    #endif

    terminated = false;
    QString old_btn_msg = ui->pushButton->text();
    ui->pushButton->setText("Terminate");
    ui->inputEdit->setEnabled(false);
    ui->returnEdit->setEnabled(false);
    ui->defaultEdit->setEnabled(false);
    ui->delimiterEdit->setEnabled(false);

    std::string r_type = ui->returnEdit->text().toStdString();
    std::string dflt = ui->defaultEdit->text().toStdString();

    SearchResult result = writePerfectHash<&callback>(keys, vals, r_type, dflt, progress, terminated, msg);

    ui->outputEdit->setText(QString::fromStdString(result.hash_source));
    ui->pushButton->setText(old_btn_msg);
    ui->inputEdit->setEnabled(true);
    ui->returnEdit->setEnabled(true);
    ui->defaultEdit->setEnabled(true);
    ui->delimiterEdit->setEnabled(true);
    terminated = true;

    callback();
}
