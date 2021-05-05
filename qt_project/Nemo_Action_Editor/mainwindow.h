#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void on_pushButton_clicked();

    void on_action_list_itemSelectionChanged();

    void on_check_path_default_toggled(bool checked);


    void on_input_path_textChanged(const QString &arg1);

    void on_check_path_prefix_toggled(bool checked);

    void on_input_create_name_textChanged(const QString &arg1);

    void on_button_add_clicked();

    void on_button_delete_selected_action_clicked();

    void on_button_duplicate_selected_action_clicked();

    void on_button_rename_clicked();

    void on_label_comment_clicked();

    void on_label_name_clicked();

    void on_label_command_clicked();

    void on_label_icon_name_clicked();

    void on_label_gtk_stock_id_clicked();

    void on_label_selection_clicked();

    void on_label_extensions_clicked();

    void on_label_mimetypes_clicked();

    void on_label_dependencies_clicked();

    void on_label_conditions_clicked();

    void on_label_separator_clicked();

    void on_label_quote_clicked();

    void on_label_escape_spaces_clicked();

    void on_label_run_in_terminal_clicked();

    void on_button_extensions_add_clicked();

    void on_button_extensions_remove_selected_clicked();

    void on_button_extensions_remove_all_clicked();

    void on_button_mimetypes_add_clicked();

    void on_button_mimetypes_remove_selected_clicked();

    void on_button_mimetypes_remove_all_clicked();

    void on_button_dependencies_add_clicked();

    void on_button_dependencies_remove_selected_clicked();

    void on_button_dependencies_remove_all_clicked();

    void on_button_save_clicked();

    void on_pushButton_2_clicked();

    void on_button_sun_clicked();

    void on_button_moon_clicked();

    void on_button_hacker_clicked();

private:
    Ui::MainWindow *ui;
    void logMessage(QString text);
    void refreshList();
    void updateEditSection();
    void updateTheme();
    void assignClass();
    void writeConf();
    void readConf();


};
#endif // MAINWINDOW_H
