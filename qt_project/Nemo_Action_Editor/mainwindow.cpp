/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * @                                        @
 * @           Nemo Action Editor           @
 * @                                        @
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */

//Qt libraries
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <QDir>
#include <QPixmap>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
using namespace std;
QString getActionPath();//return path of action's directory
void logMessage(QString text);//logs message into log window
QString getLine(QString text, int index);

QString selectedAction = "";
QString lastRefreshPath = "";//action's directory path used in last refresh
QString actionPath = "/.local/share/nemo/actions/";//default action's path, can be change thru UI
bool defaultActionPath = true;//use default action's path
bool prefixActionPath = true;//prefix action's path with user home directory (/home/user)

//file content variables
//all possible parameters in nemo_action file
//Link to sample file:
//https://github.com/linuxmint/nemo/blob/master/files/usr/share/nemo/actions/sample.nemo_action
bool action_active = false;//Active
QString action_name = "";//Name
QString action_comment = "";//Comment
QString action_command = "";//Exec
QString action_icon_name = "";//Icon-Name
QString action_GTK_ID = "";//Stock-Id
QString action_selection = "";//Selection
QString action_extensions = "";//Extension
QString action_mimetypes = "";//Mimetypes
QString action_dependencies = "";//Dependencies
QString action_conditions = "";//Conditions
QString action_separator = "";//Separator
QString action_quote = "";//Quote
bool action_escape_spaces = false;//EscapeSpaces
bool action_run_in_terminal = false;//Terminal

int theme = 0;//0 - light, 1 - dark

QString conf_path = "/.nemo_action_editor";
QString conf_name = "conf";

void MainWindow::readConf(){
    QString path = QDir::homePath() + conf_path + "/" + conf_name;
    QFile confFile(path);
    if(confFile.exists()){
        QFile f(path);
        f.open(QIODevice::ReadOnly);
        QTextStream t(&f);
        QString text = t.readAll();
        f.close();
        bool go = true;
        int index = 0;
        QString line = getLine(text, index);
        while(go){
            if(line != ""){
                if(line.startsWith("path=")){
                    ui->input_path->setText(line.mid(5));
                }else if(line.startsWith("use_default=")){
                    if(line.mid(12) == "true"){
                        ui->check_path_default->setChecked(true);
                    }else{
                        ui->check_path_default->setChecked(false);
                    }
                }else if(line.startsWith("prefix_path=")){
                    if(line.mid(12) == "true"){
                        ui->check_path_prefix->setChecked(true);
                    }else{
                        ui->check_path_prefix->setChecked(false);
                    }
                }else if(line.startsWith("theme=")){
                    theme = line.mid(6).toInt();
                    updateTheme();
                }
                index++;
                line = getLine(text, index);
            }else{
                go = false;
            }
        }
        logMessage("Config loaded");
    }else{

        QDir confDir(QDir::homePath() + conf_path);
        if(!confDir.exists()){
            confDir.mkdir(confDir.absolutePath());
        }
        QFile f(confDir.absolutePath() + "/" + conf_name);
        f.open(QIODevice::ReadWrite);
        QTextStream t(&f);
        t << QStringLiteral("path=%1\n").arg(actionPath);
        t << QStringLiteral("use_default=%1\n").arg(ui->check_path_default->isChecked() ? "true" : "false");
        t << QStringLiteral("prefix_path=%1\n").arg(ui->check_path_prefix->isChecked() ? "true" : "false");
        t << QStringLiteral("theme=%1\n").arg(theme);
        f.close();
        logMessage("Config file created");
    }

}
void MainWindow::writeConf(){
    QString text = "";
    text += QStringLiteral("path=%1\n").arg(actionPath);
    text += QStringLiteral("use_default=%1\n").arg(ui->check_path_default->isChecked() ? "true" : "false");
    text += QStringLiteral("prefix_path=%1\n").arg(ui->check_path_prefix->isChecked() ? "true" : "false");
    text += QStringLiteral("theme=%1\n").arg(theme);
    QFile f(QDir::homePath() + conf_path + "/" + conf_name);
    f.open(QIODevice::ReadWrite);
    QTextStream t(&f);
    t << text;
    f.close();
    logMessage("Config saved");
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);//create UI
    readConf();
    refreshList();//refresh actions list on load


}
MainWindow::~MainWindow()
{
    writeConf();
    delete ui;//close
}

//##############################################
//########## Action selection changed ##########
//##############################################
QString getLine(QString text, int index){//return n-th line of text
    QString line = "";
    int i = 0;
    int max = text.length();
    for(int x = 0; x <= index; x++){
        line = "";
        while(i < max && text[i] != '\n'){
            line += text[i];
            i++;
        }
        i++;
    }
    return line;
}
void MainWindow::updateEditSection(){//called when user select action on actions list
    QString path = lastRefreshPath + selectedAction + ".nemo_action";//make path to selected file

    QFile actionFile(path);//read file
    actionFile.open(QIODevice::ReadOnly);
    QTextStream f(&actionFile);
    QString text = f.readAll();
    actionFile.close();

    bool fileOK = false;
    int valuesRead = 0;
    if(getLine(text, 0) == "[Nemo Action]"){//check if file has [Nemo Action] section
        fileOK = true;
    }
    action_active = false;//reset all variables to default
    action_name = "";
    action_comment = "";
    action_command = "";
    action_icon_name = "";
    action_GTK_ID = "";
    action_selection = "";
    action_extensions = "";
    action_mimetypes = "";
    action_dependencies = "";
    action_conditions = "";
    action_separator = "";
    action_quote = "";
    action_escape_spaces = false;
    action_run_in_terminal = false;

    if(fileOK){//when file have [Nemo Action] section
        int i = 1;
        QString line = getLine(text, 1);//read file line by line
        while(line != ""){
            line = getLine(text, i);
            if(line != ""){//ingnore empty lines
                if(line[0] != '#'){//and ignore comments
                    if(line.startsWith("Active=")){//assign values from file to variables
                        if(line.mid(7) == "true"){
                            action_active = true;
                        }else{
                            action_active = false;
                        }
                        valuesRead++;
                    }else if(line.startsWith("Name=")){
                        action_name = line.mid(5);
                        valuesRead++;
                    }else if(line.startsWith("Comment=")){
                        action_comment = line.mid(8);
                        valuesRead++;
                    }else if(line.startsWith("Exec=")){
                        action_command = line.mid(5);
                        valuesRead++;
                    }else if(line.startsWith("Icon-Name=")){
                        action_icon_name = line.mid(10);
                        valuesRead++;
                    }else if(line.startsWith("Stock-Id=")){
                        action_GTK_ID = line.mid(9);
                        valuesRead++;
                    }else if(line.startsWith("Selection=")){
                        action_selection = line.mid(10);
                        valuesRead++;
                    }else if(line.startsWith("Extensions=")){
                        action_extensions = line.mid(11);
                        valuesRead++;
                    }else if(line.startsWith("Mimetypes=")){
                        action_mimetypes = line.mid(10);
                        valuesRead++;
                    }else if(line.startsWith("Separator=")){
                        action_separator = line.mid(10);
                        valuesRead++;
                    }else if(line.startsWith("Quote=")){
                        action_quote = line.mid(6);
                        valuesRead++;
                    }else if(line.startsWith("Dependencies=")){
                        action_dependencies = line.mid(13);
                        valuesRead++;
                    }else if(line.startsWith("Conditions=")){
                        action_conditions = line.mid(11);
                        valuesRead++;
                    }else if(line.startsWith("EscapeSpaces=")){
                        if(line.mid(13) == "true"){
                            action_escape_spaces = true;
                        }else{
                            action_escape_spaces = false;
                        }
                        valuesRead++;
                    }else if(line.startsWith("Terminal=")){
                        if(line.mid(9) == "true"){
                            action_run_in_terminal = true;
                        }else{
                            action_run_in_terminal = false;
                        }
                        valuesRead++;
                    }
                }
            }
            i++;
        }
    }else{//when file don't have [Nemo Action] section and file exists display error message
        if(QFile::exists(path)){//this check is because when user selects action(selectedAction is assigned) and deletes it, list refreshes and calls this function, which reads nothing from file and makes this error appear.
            QMessageBox::critical(this, "File read error", "File don't have '[Nemo Action]' section. File is broken or has been edited. Delete action and create it again.");
            return;
        }
    }

    //set loaded values to ui elements
    ui->check_active->setChecked(action_active);
    ui->input_name->setText(action_name);
    ui->input_comment->setText(action_comment);
    ui->input_command->setText(action_command);
    ui->input_icon_name->setText(action_icon_name);
    ui->input_GTK_Stock_ID->setText(action_GTK_ID);

    //in case of radio button, set all to false and then set only one to true by using if/else
    ui->radio_selection_single->setChecked(false);
    ui->radio_selection_multiple->setChecked(false);
    ui->radio_selection_any->setChecked(false);
    ui->radio_selection_notnone->setChecked(false);
    ui->radio_selection_none->setChecked(false);
    ui->radio_selection_number->setChecked(false);
    if(action_selection == "s")ui->radio_selection_single->setChecked(true);
    else if(action_selection == "m")ui->radio_selection_multiple->setChecked(true);
    else if(action_selection == "notnone")ui->radio_selection_notnone->setChecked(true);
    else if(action_selection == "none")ui->radio_selection_none->setChecked(true);
    else if(action_selection == "any")ui->radio_selection_any->setChecked(true);
    else{
        ui->radio_selection_number->setChecked(true);
        ui->input_selection_number->setValue(action_selection.toInt());
    }
    //in case of list clear them
    ui->list_extensions->clear();
    ui->list_mimetypes->clear();

    if(action_extensions != ""){
        ui->radio_extensions->setChecked(true);
        ui->radio_minetypes->setChecked(false);
        ui->radio_extensions_directory->setChecked(false);
        ui->radio_extensions_none->setChecked(false);
        ui->radio_extensions_not_directory->setChecked(false);
        ui->radio_extensions_any->setChecked(false);
        ui->radio_extensions_list->setChecked(false);
        if(action_extensions == "dir;")ui->radio_extensions_directory->setChecked(true);
        else if(action_extensions == "none;")ui->radio_extensions_none->setChecked(true);
        else if(action_extensions == "nodirs;")ui->radio_extensions_not_directory->setChecked(true);
        else if(action_extensions == "any;")ui->radio_extensions_any->setChecked(true);
        else{
            ui->radio_extensions_list->setChecked(true);
            QStringList items;
            items = action_extensions.split(";");
            for(int i = 0; i < items.size(); i++){//and add all items
                if(items[i] != ""){
                    ui->list_extensions->addItem(items[i]);
                }
            }
        }
    }else if(action_mimetypes != ""){
        ui->radio_extensions->setChecked(false);
        ui->radio_minetypes->setChecked(true);
        QStringList items;
        items = action_mimetypes.split(";");
        for(int i = 0; i < items.size(); i++){
            if(items[i] != ""){
                ui->list_mimetypes->addItem(items[i]);
            }
        }
    }
    ui->list_dependencies->clear();
    if(action_dependencies != ""){
        QStringList items;
        items = action_dependencies.split(";");
        for(int i = 0; i < items.size(); i++){
            if(items[i] != ""){
                ui->list_dependencies->addItem(items[i]);
            }
        }
    }

    ui->radio_conditions_none->setChecked(false);
    ui->radio_conditions_desktop->setChecked(false);
    ui->radio_conditions_removable->setChecked(false);
    ui->radio_conditions_gsettings->setChecked(false);
    ui->radio_conditions_dbus->setChecked(false);
    ui->radio_conditions_exec->setChecked(false);
    if(action_conditions == "none")ui->radio_conditions_none->setChecked(true);
    else if(action_conditions == "desktop")ui->radio_conditions_desktop->setChecked(true);
    else if(action_conditions == "gsettings"){
        ui->radio_conditions_gsettings->setChecked(true);
        ui->input_conditions_gsettings->setText(action_conditions.mid(10));
    }
    else if(action_conditions == "dbus"){
        ui->radio_conditions_dbus->setChecked(true);
        ui->input_conditions_dbus->setText(action_conditions.mid(5));
    }
    else if(action_conditions == "exec"){
        ui->radio_conditions_desktop->setChecked(true);
        ui->input_conditions_exec->setText(action_conditions.mid(5));
    }
    else if(action_conditions == "removable")ui->radio_conditions_removable->setChecked(true);

    ui->input_separator->setText(action_separator);

    ui->radio_quote_none->setChecked(false);
    ui->radio_quote_single->setChecked(false);
    ui->radio_quote_double->setChecked(false);
    ui->radio_quote_backtick->setChecked(false);
    if(action_quote == "none")ui->radio_quote_none->setChecked(true);
    else if(action_quote == "single")ui->radio_quote_single->setChecked(true);
    else if(action_quote == "double")ui->radio_quote_double->setChecked(true);
    else if(action_quote == "backtick")ui->radio_quote_backtick->setChecked(true);

    ui->check_escape_spaces->setChecked(action_escape_spaces);
    ui->check_run_in_terminal->setChecked(action_run_in_terminal);
    logMessage(QStringLiteral("Loaded '%1' action").arg(selectedAction));//log message to log window
}
void MainWindow::on_action_list_itemSelectionChanged()
{
    selectedAction = ui->action_list->currentItem()->text();//copy selected item to selectedAction string
    updateEditSection();
}

//#############################
//########## Refresh ##########
//#############################
void MainWindow::refreshList(){
    lastRefreshPath = getActionPath();
    QDir dir(lastRefreshPath);//directory where action files are located
    if(!dir.exists()){//when directory not exists display message box with error
        QMessageBox::critical(this, "Nemo Action Editor", "Can't find " + dir.absolutePath() + " directory. Check if path is correct.");
        return;
    }
    QFileInfoList list = dir.entryInfoList();//get list of files in directory
    ui->action_list->clear();//clear action list

    for(int i = 0; i < list.size(); ++i){//for every file in directory
        QFileInfo fileInfo = list.at(i);
        QString name = fileInfo.completeBaseName();//get it's name
        QString ext = fileInfo.completeSuffix();//and it's extension
        if(ext == "nemo_action"){//if extension is "nemo_action" add to action list
            ui->action_list->addItem(name);
        }
    }

    qDebug() << "Done";//debug info

}
void MainWindow::on_pushButton_clicked()//update action list
{
    refreshList();
    logMessage("List refreshed");
}
//###################################
//########## action's path ##########
//###################################
void MainWindow::on_check_path_default_toggled(bool checked)
{
    defaultActionPath = checked;
    if(checked){
        ui->input_path->setEnabled(false);
        ui->check_path_prefix->setEnabled(false);
    }else{
        ui->input_path->setEnabled(true);
        ui->check_path_prefix->setEnabled(true);
    }
}

void MainWindow::on_input_path_textChanged(const QString &arg1)
{
    actionPath = ui->input_path->text();
}

void MainWindow::on_check_path_prefix_toggled(bool checked)
{
    prefixActionPath = checked;
}
QString getActionPath(){
    if(defaultActionPath){
        return (QDir::homePath() + "/.local/share/nemo/actions/");
    }else{
        if(prefixActionPath){
            return (QDir::homePath() + actionPath);
        }else{
            return actionPath;
        }
    }
}
//###############################
//########## Functions ##########
//###############################
void MainWindow::logMessage(QString text){//logs message to log window and qDebug
    ui->log->append(text);
    qDebug() << text;
}
void writeDefault(QTextStream &stream){//writes default action's file content to text stream
    stream << "[Nemo Action]\n";
    stream << "# This file was generated using Nemo Action Editor\n";
    stream << "# Do not edit this file manually!\n";
    stream << "Active=true\n";
    stream << "Name=untitled\n";
    stream << "Comment=\n";
    stream << "Exec=\n";
    stream << "Icon-Name=\n";
    stream << "Stock-Id=\n";
    stream << "Selection=any\n";
    stream << "Extensions=\n";
    stream << "Mimetypes=\n";
    stream << "Separator=\n";
    stream << "Quote=\n";
    stream << "Dependencies=\n";
    stream << "Conditions=none\n";
    stream << "EscapeSpaces=false\n";
    stream << "Terminal=false\n";
}
//###########################
//########## Slots ##########
//###########################


//Add action
void MainWindow::on_input_create_name_textChanged(QString const&){}
void MainWindow::on_button_add_clicked()
{

    QString name = ui->input_create_name->text();
    if(name != ""){
        QString path = lastRefreshPath + name + ".nemo_action";
        if(QFile::exists(path)){
            QMessageBox::information(this, "Action exists", "This action already exists, rename and try again.");
            return;
        }else{
            QFile f(path);
            if(f.open(QIODevice::ReadWrite)){
                QTextStream stream(&f);
                //write default content to file
                writeDefault(stream);
                f.close();
                logMessage(QStringLiteral("Created '%1' action.").arg(name));
                refreshList();//refresh action list
            }else{
                QMessageBox::critical(this, "File error", "Error opening action file");
            }
        }
    }else{
        QMessageBox::information(this, "Enter action name", "Action name is emty, enter it and try again.");
        return;
    }

}
//delete selected action
void MainWindow::on_button_delete_selected_action_clicked()
{
    if(selectedAction == ""){
        QMessageBox::information(this, "Delete action", "Fist select action to delete.");
        return;
    }else{
        if(!QFile::exists(lastRefreshPath + selectedAction + ".nemo_action")){
            QMessageBox::information(this, "Delete action", "Fist select action to delete.");
            return;
        }else{
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Delete action", QStringLiteral("Are you sure to delete '%1' action?").arg(selectedAction), QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::Yes){
                QFile::remove(lastRefreshPath + selectedAction + ".nemo_action");
                logMessage(QStringLiteral("Deleted '%1' action").arg(selectedAction));
                refreshList();
            }
        }

    }
}
//duplicate selected action
void MainWindow::on_button_duplicate_selected_action_clicked()
{
    QString newName = ui->input_create_name->text();
    QString sourcePath = lastRefreshPath + selectedAction + ".nemo_action";
    QString targetPath = lastRefreshPath + newName + ".nemo_action";
    if(selectedAction == ""){
        QMessageBox::information(this, "Duplicate action", "First select action to duplicate");
        return;
    }
    if(newName != ""){
        if(QFile::exists(targetPath)){
            QMessageBox::information(this, "Duplicate action", "Action with given name already exists, enter another name and try again.");
            return;
        }
        if(!QFile::exists(sourcePath)){
            QMessageBox::critical(this, "Duplicate action", "Source file not exists.");
            return;
        }
        QFile source(sourcePath);
        QFile target(targetPath);
        source.open(QIODevice::ReadOnly);
        target.open(QIODevice::ReadWrite);
        QTextStream sourceStream(&source);
        QTextStream targetStream(&target);
        targetStream << sourceStream.readAll();
        source.close();
        target.close();
        logMessage(QStringLiteral("Duplicated '%1' as '%2'").arg(selectedAction).arg(newName));
        refreshList();
    }else{
        QMessageBox::information(this, "Duplicate action", "First enter name for duplicated action.");
        return;
    }

}
//rename selected
void MainWindow::on_button_rename_clicked()
{
    QString name = ui->input_create_name->text();
    QString oldName = lastRefreshPath + selectedAction + ".nemo_action";
    QString newName = lastRefreshPath + name + ".nemo_action";
    if(selectedAction == ""){
        QMessageBox::information(this, "Rename action", "First select action to rename");
        return;
    }
    if(name != ""){
        if(!QFile::exists(oldName)){
            QMessageBox::critical(this, "Rename action", "Source file not exists");
            return;
        }
        if(QFile::exists(newName)){
            QMessageBox::information(this, "Rename action", "Action with this name already exists, enter another name and try again.");
            return;
        }
        QFile::rename(oldName, newName);
        logMessage(QStringLiteral("Renamed '%1' to '%2'").arg(selectedAction).arg(name));
        refreshList();
    }else{
        QMessageBox::information(this, "Rename action", "First enter name");
    }
}
//########################################
//########## Help message boxes ##########
//########################################
void MainWindow::on_label_comment_clicked()
{
    QMessageBox::information(this, "Help - Comment", "Comment is a tool tip, appears in the status bar.");
}
void MainWindow::on_label_name_clicked()
{
    QMessageBox::information(this, "Help - Name", "Name is displayed in the menu");
}
void MainWindow::on_label_command_clicked()
{
    QMessageBox::information(this, "Help - Command", "Command is executed when user clicks on action in menu. If program resides in action's directory enclose in < >.");
}
void MainWindow::on_label_icon_name_clicked()
{
    QMessageBox::information(this, "Help - Icon name", "Icon name is a name of system theme icon. If you want to use non-system icon, copy image file to ~/.icons/ and enter file name without extension as icon name. If icon is not visible close all Nemo windows and open it again.");
}
void MainWindow::on_label_gtk_stock_id_clicked()
{
    QMessageBox::information(this, "Help - GTK Stock ID", "When GTK Stock ID and Icon name is entered GTK Stock ID is used as icon. Leave it empty to use Icon name.");
}
void MainWindow::on_label_selection_clicked()
{
    QMessageBox::information(this, "Help - Selection", "Determines when the action will appear in the menu depending on the selection. It can be:\n- single - only one element selected\n- multiple - more than one element selected\n- any - selection don't matter\n- notnone - one or more elements\n- none - no elements selected\n- number - specified number of elements selected");
}
void MainWindow::on_label_extensions_clicked()
{
    QMessageBox::information(this, "Help - Extension", "Determines when the action will appear in the menu depending on the extension. It can be:\n- directory - only directories\n- none - files with no extension\n- not directory - everything not including directories\n- any - all files and all directories\n- list - only specified ones(enter extension and click Add to add it to the extension list)");
}
void MainWindow::on_label_mimetypes_clicked()
{
    QMessageBox::information(this, "Help - Mime-types", "Determines when the action will appear in the menu depending on the mime-type. Enter the mime-type and click Add to add it to the list");
}
void MainWindow::on_label_dependencies_clicked()
{
    QMessageBox::information(this, "Help - Dependencies", "Nemo will check before displaying it in the menu, that some program exists(dependency) or not exists(reverse dependency). Enter program name and click Add tp add it to the list(to make reverse dependency prefix it with '!')");
}
void MainWindow::on_label_conditions_clicked()
{
    QMessageBox::information(this, "Help - Conditions", "Show action in the menu when:\n- none - action will alway bo shown\n- desktop - action will show only on desktop\n- gsettings - true will be returned\n- dbus - name will exists\n- exec - program will return 0");
}
void MainWindow::on_label_separator_clicked()
{
    QMessageBox::information(this, "Help - Separator", "When multiple elements are selected paths will be separated by given character. Default is ' '");
}
void MainWindow::on_label_quote_clicked()
{
    QMessageBox::information(this, "Help - Quote", "Enclose paths in selected quotes.\n- none - /path/to/file\n- single - '/path/to/file'\n- double - \"/path/to/file\"\n- backtick - `/path/to/file`");
}
void MainWindow::on_label_escape_spaces_clicked()
{
    QMessageBox::information(this, "Help - Escape spaces", "When checked all spaces in paths will be escaped(' ' -> '\\ ')");
}
void MainWindow::on_label_run_in_terminal_clicked()
{
    QMessageBox::information(this, "Help - Run in terminal", "If checked new terminal window will be spawned and command will be executed in that window.");
}
void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox::information(this, "Help - Additional information", "Syntax used in Name, Comment and Exec fiels:\n%U - URI list of selected elements\n%F - path list of selected elements\n%P - path of parent(current) directory\n %f or %N - display name of first selected file\n%p - display name of parent directory\n%D - device path of file (i.e. /dev/sdb1)\n%e - display name of first selected file without extension\n%% - '%' character\n%X - XID of Nemo window\n\n'Command' and 'Conditions exec' can be put inside < > if the program is located in action's directory\nClick on labels or '?' for more information\nSample nemo action's file with description: 'https://github.com/linuxmint/nemo/blob/master/files/usr/share/nemo/actions/sample.nemo_action'\n\nFields with red labels are required!!!");
}
//#############################################
//########## Lists control functions ##########
//#############################################
//extensions
void MainWindow::on_button_extensions_add_clicked()
{
    QString text = ui->input_add_extensions_list->text();
    if(text != ""){
        QList<QListWidgetItem *> list = ui->list_extensions->findItems(text, Qt::MatchFixedString);
        int count = 0;
        for(QListWidgetItem *item : list){
            count++;
        }
        if(count == 0){
            ui->list_extensions->addItem(text);
            ui->input_add_extensions_list->clear();
        }else{
            QMessageBox::information(this, "Add item", "This item already exists");
        }
    }
}
void MainWindow::on_button_extensions_remove_selected_clicked()
{
    qDeleteAll(ui->list_extensions->selectedItems());
}
void MainWindow::on_button_extensions_remove_all_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete all items", "Are you sure to delete all items?", QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes){
        ui->list_extensions->clear();
    }
}
//mimetypes
void MainWindow::on_button_mimetypes_add_clicked()
{
    QString text = ui->input_add_mimetypes_list->text();
    if(text != ""){
        QList<QListWidgetItem *> list = ui->list_mimetypes->findItems(text, Qt::MatchFixedString);
        int count = 0;
        for(QListWidgetItem *item : list){
            count++;
        }
        if(count == 0){
            ui->list_mimetypes->addItem(text);
            ui->input_add_mimetypes_list->clear();
        }else{
            QMessageBox::information(this, "Add item", "This item already exists");
        }
    }
}
void MainWindow::on_button_mimetypes_remove_selected_clicked()
{
    qDeleteAll(ui->list_mimetypes->selectedItems());
}
void MainWindow::on_button_mimetypes_remove_all_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete all items", "Are you sure to delete all items?", QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes){
        ui->list_mimetypes->clear();
    }
}
//dependencies
void MainWindow::on_button_dependencies_add_clicked()
{
    QString text = ui->input_add_dependencies_list->text();
    if(text != ""){
        QList<QListWidgetItem *> list = ui->list_dependencies->findItems(text, Qt::MatchFixedString);
        int count = 0;
        for(QListWidgetItem *item : list){
            count++;
        }
        if(count == 0){
            ui->list_dependencies->addItem(text);
            ui->input_add_dependencies_list->clear();
        }else{
            QMessageBox::information(this, "Add item", "This item already exists");
        }
    }
}
void MainWindow::on_button_dependencies_remove_selected_clicked()
{
    qDeleteAll(ui->list_dependencies->selectedItems());
}
void MainWindow::on_button_dependencies_remove_all_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete all items", "Are you sure to delete all items?", QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes){
        ui->list_dependencies->clear();
    }
}
//##########################
//########## SAVE ##########
//##########################
void MainWindow::on_button_save_clicked()
{
    if(selectedAction != ""){
        action_active = ui->check_active->isChecked();
        action_name = ui->input_name->text();
        action_comment = ui->input_comment->text();
        action_command = ui->input_command->text();
        action_icon_name = ui->input_icon_name->text();
        action_GTK_ID = ui->input_GTK_Stock_ID->text();

        action_selection = "";
        if(ui->radio_selection_single->isChecked())action_selection = "s";
        else if(ui->radio_selection_multiple->isChecked())action_selection = "m";
        else if(ui->radio_selection_none->isChecked())action_selection = "none";
        else if(ui->radio_selection_notnone->isChecked())action_selection = "notnone";
        else if(ui->radio_selection_any->isChecked())action_selection = "any";
        else if(ui->radio_selection_number->isChecked())action_selection = QStringLiteral("%1").arg(ui->input_selection_number->value());

        action_mimetypes = "";
        action_extensions = "";
        if(ui->radio_extensions->isChecked()){
            if(ui->radio_extensions_directory->isChecked())action_extensions = "dir;";
            else if(ui->radio_extensions_none->isChecked())action_extensions = "none;";
            else if(ui->radio_extensions_not_directory->isChecked())action_extensions = "nodirs;";
            else if(ui->radio_extensions_any->isChecked())action_extensions = "any;";
            else if(ui->radio_extensions_list->isChecked()){
                for(int i = 0; i < ui->list_extensions->count(); i++){
                    action_extensions += ui->list_extensions->item(i)->text();
                    action_extensions += ";";
                }
            }
        }else{
            for(int i = 0; i < ui->list_mimetypes->count(); i++){
                action_mimetypes += ui->list_mimetypes->item(i)->text();
                action_mimetypes += ";";
            }
        }

        action_dependencies = "";
        for(int i = 0; i < ui->list_dependencies->count(); i++){
            action_dependencies += ui->list_dependencies->item(i)->text();
            action_dependencies += ";";
        }

        action_conditions = "";
        if(ui->radio_conditions_none->isChecked())action_conditions = "none";
        else if(ui->radio_conditions_desktop->isChecked())action_conditions = "desktop";
        else if(ui->radio_conditions_removable->isChecked())action_conditions = "removable";
        else if(ui->radio_conditions_gsettings->isChecked())action_conditions = QStringLiteral("gsettings %1").arg(ui->input_conditions_gsettings->text());
        else if(ui->radio_conditions_dbus->isChecked())action_conditions = QStringLiteral("dbus %1").arg(ui->input_conditions_dbus->text());
        else if(ui->radio_conditions_exec->isChecked())action_conditions = QStringLiteral("exec %1").arg(ui->input_conditions_exec->text());

        action_separator = ui->input_separator->text();

        action_quote = "";
        if(ui->radio_quote_none->isChecked())action_quote = "";
        else if(ui->radio_quote_single->isChecked())action_quote = "single";
        else if(ui->radio_quote_double->isChecked())action_quote = "double";
        else if(ui->radio_quote_backtick->isChecked())action_quote = "backtick";

        action_escape_spaces = ui->check_escape_spaces->isChecked();
        action_run_in_terminal = ui->check_run_in_terminal->isChecked();

        if(action_name == ""){
            QMessageBox::information(this, "Save - Name", "Action name is required to save.");
            return;
        }
        if(action_command == ""){
            QMessageBox::information(this, "Save - Command", "Action command is required to save.");
            return;
        }
        QString toFile = "[Nemo Action]\n# This file was generated using Nemo Action Editor\n# Do not edit this file manually!\n";
        toFile += QStringLiteral("Active=%1\n").arg(action_active ? "true" : "false");
        toFile += QStringLiteral("Name=%1\n").arg(action_name);
        toFile += QStringLiteral("Comment=%1\n").arg(action_comment);
        toFile += QStringLiteral("Exec=%1\n").arg(action_command);
        toFile += QStringLiteral("Icon-Name=%1\n").arg(action_icon_name);
        toFile += QStringLiteral("Stock-Id=%1\n").arg(action_GTK_ID);
        toFile += QStringLiteral("Selection=%1\n").arg(action_selection);
        toFile += QStringLiteral("Extensions=%1\n").arg(action_extensions);
        toFile += QStringLiteral("Mimetypes=%1\n").arg(action_mimetypes);
        toFile += QStringLiteral("Separator=%1\n").arg(action_separator);
        toFile += QStringLiteral("Quote=%1\n").arg(action_quote);
        toFile += QStringLiteral("Dependencies=%1\n").arg(action_dependencies);
        toFile += QStringLiteral("Conditions=%1\n").arg(action_conditions);
        toFile += QStringLiteral("EscapeSpaces=%1\n").arg(action_escape_spaces ? "true" : "false");
        toFile += QStringLiteral("Terminal=%1\n").arg(action_run_in_terminal ? "true" : "false");

        QFile f(lastRefreshPath + selectedAction + ".nemo_action");
        f.open(QIODevice::ReadWrite);

        QTextStream t(&f);
        t << toFile;
        f.close();
        logMessage(QStringLiteral("Saved '%1' action").arg(selectedAction));
    }else{
        QMessageBox::information(this, "Save", "Select action to save.");
    }
}
//###########################
//########## THEME ##########
//###########################
void MainWindow::on_button_sun_clicked()
{
    theme = 0;
    updateTheme();
}

void MainWindow::on_button_moon_clicked()
{
    theme = 1;
    updateTheme();
}
void MainWindow::on_button_hacker_clicked()
{
    theme = 2;
    updateTheme();
}
void MainWindow::updateTheme(){
    if(theme == 0){
        QFile f(":/theme/light");
        f.open(QIODevice::ReadOnly);
        QTextStream t(&f);
        QString text = t.readAll();
        f.close();

        qApp->setStyleSheet(text);
        ui->button_sun->setIcon(QIcon(":/images/sun-black"));
        ui->button_moon->setIcon(QIcon(":/images/moon-black"));
        ui->button_hacker->setIcon(QIcon(":/images/hacker-black"));
        ui->button_duplicate_selected_action->setIcon(QIcon(":/images/copy.png"));
        ui->button_rename->setIcon(QIcon(":/images/rename.png"));
    }else if(theme == 1){
        QFile f(":/theme/dark");
        f.open(QIODevice::ReadOnly);
        QTextStream t(&f);
        QString text = t.readAll();
        f.close();

        qApp->setStyleSheet(text);
        ui->button_sun->setIcon(QIcon(":/images/sun-white"));
        ui->button_moon->setIcon(QIcon(":/images/moon-white"));
        ui->button_hacker->setIcon(QIcon(":/images/hacker-white"));
        ui->button_duplicate_selected_action->setIcon(QIcon(":/images/copy2"));
        ui->button_rename->setIcon(QIcon(":/images/rename2"));
    }else if(theme == 2){
        QFile f(":/theme/hacker");
        f.open(QIODevice::ReadOnly);
        QTextStream t(&f);
        QString text = t.readAll();
        f.close();

        qApp->setStyleSheet(text);
        ui->button_sun->setIcon(QIcon(":/images/sun-green"));
        ui->button_moon->setIcon(QIcon(":/images/moon-green"));
        ui->button_hacker->setIcon(QIcon(":/images/hacker-green"));
        ui->button_duplicate_selected_action->setIcon(QIcon(":/images/copy3"));
        ui->button_rename->setIcon(QIcon(":/images/rename3"));
    }
}


