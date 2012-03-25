/*
    hashcat-gui - GUI for the hashcat tools
    Copyright (C) 2012 Rainer Größlinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QComboBox>
#include <QProcess>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_actionHelp_About_triggered();

    void on_pushButton_hc_open_hashfile_clicked();

    void on_pushButton_hc_open_rulesfile_clicked();

    void on_pushButton_hc_add_wordlist_clicked();

    void on_listWidget_hc_wordlist_itemClicked(QListWidgetItem* item);

    void on_pushButton_hc_remove_wordlist_clicked();

    void on_pushButton_hc_execute_clicked();

    void on_pushButton_hc_open_saltfile_clicked();

    void on_pushButton_oclhcplus_execute_clicked();

    void on_pushButton_oclhcplus_open_hashfile_clicked();

    void on_pushButton_hc_open_tablefile_clicked();

    void on_actionReset_fields_triggered();

    void on_comboBox_hc_hash_currentIndexChanged(QString );

    void on_actionQuit_triggered();

    void on_actionHashcat_EULA_triggered();

    void on_actionOclHashcatPlus_EULA_triggered();

    void on_actionOclHashcatLite_EULA_triggered();

    void on_toolButton_hc_wordlist_sort_asc_clicked();

    void on_toolButton_hc_wordlist_sort_desc_clicked();

    void on_pushButton_oclhclite_execute_clicked();

    void on_pushButton_hc_output_clicked();

    void on_pushButton_oclhcplus_output_clicked();

    void on_pushButton_oclhclite_output_clicked();

    void on_pushButton_oclhcplus_remove_wordlist_clicked();

    void on_pushButton_oclhcplus_add_wordlist_clicked();

    void on_toolButton_oclhcplus_wordlist_sort_asc_clicked();

    void on_toolButton_oclhcplus_wordlist_sort_desc_clicked();

    void on_listWidget_oclhcplus_wordlist_itemClicked(QListWidgetItem* item);

    void on_checkBox_hc_saltfile_toggled(bool checked);

    void on_checkBox_hc_outfile_toggled(bool checked);

    void on_checkBox_oclhcplus_outfile_toggled(bool checked);

    void on_checkBox_oclhclite_outfile_toggled(bool checked);

    void on_checkBox_oclhclite_mask_toggled(bool checked);

    void on_checkBox_oclhclite_custom1_toggled(bool checked);

    void on_checkBox_oclhclite_custom2_toggled(bool checked);

    void on_checkBox_oclhclite_custom3_toggled(bool checked);

    void on_checkBox_oclhclite_custom4_toggled(bool checked);

    void on_pushButton_oclhcplus_add_wordlist_folder_clicked();

    void on_pushButton_hc_add_wordlist_folder_clicked();

    void on_radioButton_hc_open_rulesfile_toggled(bool checked);

    void on_radioButton_hc_rules_generated_toggled(bool checked);

    void hcCommandChanged(QString arg="");

    void oclhcplusCommandChanged(QString arg="");

    void oclhcliteCommandChanged(QString arg="");

    void on_comboBox_hc_attack_currentIndexChanged(int index);

    void on_comboBox_oclhcplus_attack_currentIndexChanged(int index);

    void on_checkBox_oclhcplus_custom1_toggled(bool checked);

    void on_checkBox_oclhcplus_custom2_toggled(bool checked);

    void on_checkBox_oclhcplus_custom3_toggled(bool checked);

    void on_checkBox_oclhcplus_custom4_toggled(bool checked);

    void on_lineEdit_hc_open_hashfile_textChanged(const QString &arg1);

    void on_lineEdit_oclhcplus_open_hashfile_textChanged(const QString &arg1);

    void on_lineEdit_oclhclite_hash_textChanged(const QString &arg1);

    void on_checkBox_oclhclite_skip_plain_toggled(bool checked);

    void on_checkBox_oclhclite_limit_plain_toggled(bool checked);

    void on_checkBox_oclhcplus_rulesfile_1_toggled(bool checked);

    void on_checkBox_oclhcplus_rulesfile_2_toggled(bool checked);

    void on_checkBox_oclhcplus_rulesfile_3_toggled(bool checked);

    void on_radioButton_oclhcplus_generate_rules_toggled(bool checked);

    void on_radioButton_oclhcplus_use_rules_file_toggled(bool checked);

    void on_pushButton_oclhcplus_open_rulesfile_1_clicked();

    void on_pushButton_oclhcplus_open_rulesfile_2_clicked();

    void on_pushButton_oclhcplus_open_rulesfile_3_clicked();

    void on_checkBox_oclhclite_skip_toggled(bool checked);

    void on_checkBox_oclhclite_limit_toggled(bool checked);

    void on_checkBox_hc_custom1_toggled(bool checked);

    void on_checkBox_hc_custom2_toggled(bool checked);

    void on_checkBox_hc_custom3_toggled(bool checked);

    void on_checkBox_hc_custom4_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    QMap <QString, QString> envInfo;
    QMap <quint32, QString> hc_hashModes, oclhcplus_hashModes, oclhclite_hashModes;
    QMap <quint32, QString> hc_attackModes, oclhcplus_attackModes;

    void init_hash_and_attack_modes();
    void hc_update_view_attack_mode();
    void oclhcplus_update_view_attack_mode();

    QStringList generate_terminal_env(qint16);
    QStringList hc_generate_arguments();
    QStringList oclhcplus_generate_arguments();
    QStringList oclhclite_generate_arguments();

    void add_hash_and_attack_modes(QComboBox *&, QMap <quint32, QString> &);
    void add_wordlist_item(QString &, qint16);
    void add_wordlist_item(QStringList &, qint16);
    void set_outfile_path(qint16);
};

#endif // MAINWINDOW_H
