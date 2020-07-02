#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT

private:
    bool valid_input = true;
    std::vector<std::string> keys;
    std::vector<std::string> vals;
    int empty_space = 8;

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    Ui::MainWindow* ui;

private slots:
    void parseInputField();
    void writeHashFunction();
};
#endif // MAINWINDOW_H
