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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "euladialog.h"
#include "aboutdialog.h"
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    envInfo.insert("wordsize", QString::number(QSysInfo::WordSize));
#if defined(Q_WS_WIN)
    envInfo.insert("terminal", "cmd.exe");
    QString fileExt = ".exe";
    envInfo.insert("dir_current", ".\\");
    envInfo.insert("dir_hc", ".\\hashcat\\");
    envInfo.insert("dir_oclhcplus", ".\\oclHashcat-plus\\");
    envInfo.insert("dir_oclhclite", ".\\oclHashcat-lite\\");
#else
    envInfo.insert("terminal", "xterm");
    QString fileExt = ".bin";
    envInfo.insert("dir_current", "./");
    envInfo.insert("dir_hc", "./hashcat/");
    envInfo.insert("dir_oclhcplus", "./oclHashcat-plus/");
    envInfo.insert("dir_oclhclite", "./oclHashcat-lite/");
#endif
    envInfo.insert("cmd_hc", "hashcat-cli" + envInfo.value("wordsize") + fileExt);

    ui->setupUi(this);

    QMessageBox msgBox(this);
    QPushButton *otherButton = msgBox.addButton(tr("CPU only"), QMessageBox::AcceptRole);
    QPushButton *nvidiaButton = msgBox.addButton(tr("NVIDIA (CUDA)"), QMessageBox::AcceptRole);
    QPushButton *amdButton = msgBox.addButton(tr("AMD (OpenCL)"), QMessageBox::AcceptRole);

    msgBox.setModal(true);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("What hardware device would you like to use?");
    msgBox.exec();

    if (msgBox.clickedButton() == amdButton) {
        envInfo.insert("cmd_oclhcplus", "oclHashcat-plus" + envInfo.value("wordsize") + fileExt);
        envInfo.insert("cmd_oclhclite", "oclHashcat-lite" + envInfo.value("wordsize") + fileExt);
        ui->tabWidget_main->setTabText(1, "oclHashcat-plus");
        ui->tabWidget_main->setTabText(2, "oclHashcat-lite");
        ui->actionOclHashcatPlus_EULA->setText("oclHashcat-plus EULA");
        ui->actionOclHashcatLite_EULA->setText("oclHashcat-lite EULA");
        ui->checkBox_oclhcplus_async->setVisible(false);
        ui->checkBox_oclhclite_async->setVisible(false);
    } else if (msgBox.clickedButton() == nvidiaButton) {
        envInfo.insert("cmd_oclhcplus", "cudaHashcat-plus" + envInfo.value("wordsize") + fileExt);
        envInfo.insert("cmd_oclhclite", "cudaHashcat-lite" + envInfo.value("wordsize") + fileExt);
        ui->tabWidget_main->setTabText(1, "cudaHashcat-plus");
        ui->tabWidget_main->setTabText(2, "cudaHashcat-lite");
        ui->actionOclHashcatPlus_EULA->setText("cudaHashcat-plus EULA");
        ui->actionOclHashcatLite_EULA->setText("cudaHashcat-lite EULA");
    } else if (msgBox.clickedButton() == otherButton) {
        ui->tab_oclhcplus->setEnabled(false);
        ui->tab_oclhclite->setEnabled(false);
        ui->actionOclHashcatPlus_EULA->setVisible(false);
        ui->actionOclHashcatLite_EULA->setVisible(false);
    }

    this->init_hash_and_attack_modes();
    this->hc_update_view_attack_mode();
    this->oclhcplus_update_view_attack_mode();

    connect(ui->listWidget_hc_wordlist->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(hcCommandChanged()));
    connect(ui->listWidget_hc_wordlist->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(hcCommandChanged()));
    connect(ui->listWidget_hc_wordlist->model(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT(hcCommandChanged()));

    connect(ui->listWidget_oclhcplus_wordlist->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(oclhcplusCommandChanged()));
    connect(ui->listWidget_oclhcplus_wordlist->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(oclhcplusCommandChanged()));
    connect(ui->listWidget_oclhcplus_wordlist->model(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT(oclhcplusCommandChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/********* MainWindow *************************************/

QStringList MainWindow::generate_terminal_env(qint16 hc_type) {

    QStringList arguments;
    QString path;

    switch(hc_type) {
    case 0: path = envInfo.value("dir_current") + envInfo.value("cmd_hc"); break;
    case 1: path = envInfo.value("dir_current") + envInfo.value("cmd_oclhcplus"); break;
    case 2: path = envInfo.value("dir_current") + envInfo.value("cmd_oclhclite"); break;
    }

#if defined(Q_WS_WIN)
    arguments << "/k" << path;
#else
    arguments << "-hold" << "-e" << path;
#endif

    return arguments;
}

void MainWindow::hcCommandChanged(QString arg) {
    ui->lineEdit_hc_command->setText(envInfo.value("cmd_hc") + " " + (arg.length() ? arg : hc_generate_arguments().join(" ")));
    ui->lineEdit_hc_command->setCursorPosition(0);
}

void MainWindow::oclhcplusCommandChanged(QString arg) {
    ui->lineEdit_oclhcplus_command->setText(envInfo.value("cmd_oclhcplus") + " " + (arg.length() ? arg : oclhcplus_generate_arguments().join(" ")));
    ui->lineEdit_oclhcplus_command->setCursorPosition(0);
}

void MainWindow::oclhcliteCommandChanged(QString arg) {
    ui->lineEdit_oclhclite_command->setText(envInfo.value("cmd_oclhclite") + " " + (arg.length() ? arg : oclhclite_generate_arguments().join(" ")));
    ui->lineEdit_oclhclite_command->setCursorPosition(0);
}

void MainWindow::on_actionReset_fields_triggered()
{
    switch(ui->tabWidget_main->currentIndex()) {
    case 0:
        ui->lineEdit_hc_open_hashfile->clear();
        ui->lineEdit_hc_separator->setText(":");
        ui->checkBox_hc_remove->setChecked(false);
        ui->listWidget_hc_wordlist->clear();
        ui->comboBox_hc_attack->setCurrentIndex(0);
        ui->comboBox_hc_hash->setCurrentIndex(0);
        ui->checkBox_hc_saltfile->setChecked(false);
        ui->lineEdit_hc_open_saltfile->clear();
        ui->radioButton_hc_rules_none->setChecked(true);
        ui->lineEdit_hc_open_rulesfile->clear();
        ui->spinBox_hc_rules->setValue(ui->spinBox_hc_rules->minimum());
        ui->spinBox_hc_toggle_min->setValue(ui->spinBox_hc_toggle_min->minimum());
        ui->spinBox_hc_toggle_max->setValue(ui->spinBox_hc_toggle_max->maximum());
        ui->spinBox_hc_password_min->setValue(ui->spinBox_hc_password_min->minimum());
        ui->spinBox_hc_password_max->setValue(ui->spinBox_hc_password_max->maximum());
        ui->lineEdit_hc_mask->clear();
        ui->checkBox_hc_custom1->setChecked(false);
        ui->lineEdit_hc_custom1->clear();
        ui->checkBox_hc_custom2->setChecked(false);
        ui->lineEdit_hc_custom2->clear();
        ui->checkBox_hc_custom3->setChecked(false);
        ui->lineEdit_hc_custom3->clear();
        ui->checkBox_hc_custom4->setChecked(false);
        ui->lineEdit_hc_custom4->clear();
        ui->lineEdit_hc_open_tablefile->clear();
        ui->checkBox_hc_potfile->setChecked(false);
        ui->checkBox_hc_outfile->setChecked(false);
        ui->lineEdit_hc_outfile->clear();
        ui->comboBox_hc_outfile_format->setCurrentIndex(0);
        ui->spinBox_hc_threads->setValue(8);
        ui->spinBox_hc_segment->setValue(32);
        ui->spinBox_hc_skip->setValue(ui->spinBox_hc_skip->minimum());
        ui->spinBox_hc_limit->setValue(ui->spinBox_hc_limit->minimum());
        this->hcCommandChanged();
        break;
    case 1:
        ui->lineEdit_oclhcplus_open_hashfile->clear();
        ui->checkBox_oclhcplus_ignoreusername->setChecked(false);
        ui->checkBox_oclhcplus_remove->setChecked(false);
        ui->listWidget_oclhcplus_wordlist->clear();
        ui->comboBox_oclhcplus_attack->setCurrentIndex(0);
        ui->comboBox_oclhcplus_hash->setCurrentIndex(0);
        ui->radioButton_oclhcplus_use_rules_file->setChecked(true);
        ui->checkBox_oclhcplus_rulesfile_1->setChecked(false);
        ui->checkBox_oclhcplus_rulesfile_2->setChecked(false);
        ui->checkBox_oclhcplus_rulesfile_3->setChecked(false);
        ui->lineEdit_oclhcplus_open_rulesfile_1->clear();
        ui->lineEdit_oclhcplus_open_rulesfile_2->clear();
        ui->lineEdit_oclhcplus_open_rulesfile_3->clear();
        ui->spinBox_oclhcplus_generate_rules->setValue(1);
        ui->spinBox_oclhcplus_password_min->setValue(ui->spinBox_oclhcplus_password_min->minimum());
        ui->spinBox_oclhcplus_password_max->setValue(ui->spinBox_oclhcplus_password_max->maximum());
        ui->lineEdit_oclhcplus_mask->clear();
        ui->checkBox_oclhcplus_custom1->setChecked(false);
        ui->lineEdit_oclhcplus_custom1->clear();
        ui->checkBox_oclhcplus_custom2->setChecked(false);
        ui->lineEdit_oclhcplus_custom2->clear();
        ui->checkBox_oclhcplus_custom3->setChecked(false);
        ui->lineEdit_oclhcplus_custom3->clear();
        ui->checkBox_oclhcplus_custom4->setChecked(false);
        ui->lineEdit_oclhcplus_custom4->clear();
        ui->checkBox_oclhcplus_hex_hash->setChecked(false);
        ui->checkBox_oclhcplus_hex_salt->setChecked(false);
        ui->spinBox_oclhcplus_password_min->setValue(1);
        ui->spinBox_oclhcplus_password_max->setValue(16);
        ui->checkBox_oclhcplus_outfile->setChecked(false);
        ui->lineEdit_oclhcplus_outfile->clear();
        ui->comboBox_oclhcplus_outfile_format->setCurrentIndex(2);
        ui->checkBox_oclhcplus_async->setChecked(false);
        ui->lineEdit_oclhcplus_cpu_affinity->clear();
        ui->lineEdit_oclhcplus_devices->setText("0");
        ui->spinBox_oclhcplus_accel->setValue(8);
        ui->spinBox_oclhcplus_loops->setValue(256);
        ui->spinBox_oclhcplus_watchdog->setValue(90);
        ui->spinBox_oclhcplus_segment->setValue(32);
        this->oclhcplusCommandChanged();
        break;
    case 2:
        ui->lineEdit_oclhclite_hash->clear();
        ui->checkBox_oclhclite_mask->setChecked(false);
        ui->lineEdit_oclhclite_mask->clear();
        ui->lineEdit_oclhclite_separator->setText(":");
        ui->comboBox_oclhclite_hash->setCurrentIndex(0);
        ui->checkBox_oclhclite_custom1->setChecked(false);
        ui->lineEdit_oclhclite_custom1->clear();
        ui->checkBox_oclhclite_custom2->setChecked(false);
        ui->lineEdit_oclhclite_custom2->clear();
        ui->checkBox_oclhclite_custom3->setChecked(false);
        ui->lineEdit_oclhclite_custom3->clear();
        ui->checkBox_oclhclite_custom4->setChecked(false);
        ui->lineEdit_oclhclite_custom4->clear();
        ui->checkBox_oclhclite_hex_hash->setChecked(false);
        ui->checkBox_oclhclite_hex_salt->setChecked(false);
        ui->spinBox_oclhclite_password_min->setValue(ui->spinBox_oclhclite_password_min->minimum());
        ui->spinBox_oclhclite_password_max->setValue(15);
        ui->checkBox_oclhclite_skip->setChecked(false);
        ui->lineEdit_oclhclite_skip->clear();
        ui->checkBox_oclhclite_limit->setChecked(false);
        ui->lineEdit_oclhclite_limit->clear();
        ui->checkBox_oclhclite_skip_plain->setChecked(false);
        ui->lineEdit_oclhclite_skip_plain->clear();
        ui->checkBox_oclhclite_limit_plain->setChecked(false);
        ui->lineEdit_oclhclite_limit_plain->clear();
        ui->checkBox_oclhclite_outfile->setChecked(false);
        ui->lineEdit_oclhclite_outfile->clear();
        ui->comboBox_oclhclite_outfile_format->setCurrentIndex(2);
        ui->checkBox_oclhclite_async->setChecked(false);
        ui->lineEdit_oclhclite_cpu_affinity->clear();
        ui->lineEdit_oclhclite_devices->setText("0");
        ui->spinBox_oclhclite_accel->setValue(8);
        ui->spinBox_oclhclite_loops->setValue(256);
        ui->spinBox_oclhclite_watchdog->setValue(90);
        this->oclhcliteCommandChanged();
        break;
    }
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionHashcat_EULA_triggered()
{
    eulaDialog *eula = new eulaDialog();
    QMap <QString, QString> &info = envInfo;
    eula->get_eula(info, 0);
    eula->show();
}

void MainWindow::on_actionOclHashcatPlus_EULA_triggered()
{
    eulaDialog *eula = new eulaDialog();
    QMap <QString, QString> &info = envInfo;
    eula->get_eula(info, 1);
    eula->show();
}

void MainWindow::on_actionOclHashcatLite_EULA_triggered()
{
    eulaDialog *eula = new eulaDialog();
    QMap <QString, QString> &info = envInfo;
    eula->get_eula(info, 2);
    eula->show();
}

void MainWindow::on_actionHelp_About_triggered()
{
    aboutDialog *about = new aboutDialog();
    QMap <QString, QString> &info = envInfo;
    about->get_versions(info);
    about->show();
}

void MainWindow::add_hash_and_attack_modes(QComboBox *&combobox, QMap <quint32, QString> &map) {
    QMapIterator<quint32, QString> iter(map);
    while (iter.hasNext()) {
        iter.next();
        combobox->addItem(iter.value());
    }
}

void MainWindow::init_hash_and_attack_modes() {
    hc_attackModes.insert(0, "Straight");
    hc_attackModes.insert(1, "Combination");
    hc_attackModes.insert(2, "Toggle-Case");
    hc_attackModes.insert(3, "Brute-force");
    hc_attackModes.insert(4, "Permutation");
    hc_attackModes.insert(5, "Table-Lookup");

    oclhcplus_attackModes.insert(0, "Straight");
    oclhcplus_attackModes.insert(1, "Combination");
    oclhcplus_attackModes.insert(3, "Brute-force");
    oclhcplus_attackModes.insert(4, "Permutation");
    oclhcplus_attackModes.insert(6, "Hybrid dict + mask");
    oclhcplus_attackModes.insert(7, "Hybrid mask + dict");

    hc_hashModes.insert(0, "MD5");
    hc_hashModes.insert(1, "md5($pass.$salt)");
    hc_hashModes.insert(2, "md5($salt.$pass)");
    hc_hashModes.insert(3, "md5(md5($pass))");
    hc_hashModes.insert(4, "md5(md5(md5($pass)))");
    hc_hashModes.insert(5, "vBulletin < v3.8.5");
    hc_hashModes.insert(6, "md5(md5($salt).$pass)");
    hc_hashModes.insert(7, "md5($salt.md5($pass))");
    hc_hashModes.insert(8, "md5($salt.$pass.$salt)");
    hc_hashModes.insert(9, "md5(md5($salt).md5($pass))");
    hc_hashModes.insert(10, "md5(md5($pass).md5($salt))");
    hc_hashModes.insert(11, "md5($salt.md5($salt.$pass))");
    hc_hashModes.insert(12, "md5($salt.md5($pass.$salt))");
    hc_hashModes.insert(15, "vBulletin > v3.8.5");
    hc_hashModes.insert(30, "md5($username.0.$pass)");
    hc_hashModes.insert(31, "md5(strtoupper(md5($pass)))");
    hc_hashModes.insert(100, "SHA1");
    hc_hashModes.insert(101, "sha1($pass.$salt)");
    hc_hashModes.insert(102, "sha1($salt.$pass)");
    hc_hashModes.insert(103, "sha1(sha1($pass))");
    hc_hashModes.insert(104, "sha1(sha1(sha1($pass)))");
    hc_hashModes.insert(105, "sha1(strtolower($username).$pass)");
    hc_hashModes.insert(200, "MySQL");
    hc_hashModes.insert(300, "MySQL4.1/MySQL5");
    hc_hashModes.insert(400, "MD5(Wordpress)");
    hc_hashModes.insert(400, "MD5(phpBB3)");
    hc_hashModes.insert(500, "MD5(Unix)");
    hc_hashModes.insert(600, "SHA-1(Base64)");
    hc_hashModes.insert(700, "SSHA-1(Base64)");
    hc_hashModes.insert(800, "SHA-1(Django)");
    hc_hashModes.insert(900, "MD4");
    hc_hashModes.insert(1000, "NTLM");
    hc_hashModes.insert(1100, "Domain Cached Credentials");
    hc_hashModes.insert(1200, "MD5(Chap)");
    hc_hashModes.insert(1300, "MSSQL");
    hc_hashModes.insert(1400, "SHA256");
    hc_hashModes.insert(1600, "MD5(APR)");
    hc_hashModes.insert(1700, "SHA512");
    hc_hashModes.insert(1800, "SHA-512(Unix)");

    oclhcplus_hashModes.insert(0, "MD5");
    oclhcplus_hashModes.insert(11, "Joomla");
    oclhcplus_hashModes.insert(21, "osCommerce, xt:Commerce");
    oclhcplus_hashModes.insert(100, "SHA1");
    oclhcplus_hashModes.insert(101, "nsldap, SHA-1(Base64), Netscape LDAP SHA");
    oclhcplus_hashModes.insert(111, "nsldaps, SSHA-1(Base64), Netscape LDAP SSHA");
    oclhcplus_hashModes.insert(112, "Oracle 11g");
    oclhcplus_hashModes.insert(121, "SMF > v1.1");
    oclhcplus_hashModes.insert(122, "OSX v10.4, v10.5, v10.6");
    oclhcplus_hashModes.insert(131, "MSSQL(2000)");
    oclhcplus_hashModes.insert(132, "MSSQL(2005)");
    oclhcplus_hashModes.insert(300, "MySQL > v4.1");
    oclhcplus_hashModes.insert(400, "phpass, MD5(Wordpress), MD5(phpBB3)");
    oclhcplus_hashModes.insert(500, "md5crypt, MD5(Unix), FreeBSD MD5, Cisco-IOS MD5");
    oclhcplus_hashModes.insert(900, "MD4");
    oclhcplus_hashModes.insert(1000, "NTLM");
    oclhcplus_hashModes.insert(1100, "Domain Cached Credentials, mscash");
    oclhcplus_hashModes.insert(1400, "SHA256");
    oclhcplus_hashModes.insert(1500, "descrypt, DES(Unix), Traditional DES");
    oclhcplus_hashModes.insert(1600, "md5apr1, MD5(APR), Apache MD5");
    oclhcplus_hashModes.insert(2100, "Domain Cached Credentials2, mscash2");
    oclhcplus_hashModes.insert(2400, "Cisco-PIX MD5");
    oclhcplus_hashModes.insert(2500, "WPA/WPA2");
    oclhcplus_hashModes.insert(2611, "vBulletin < v3.8.5");
    oclhcplus_hashModes.insert(2711, "vBulletin > v3.8.5");
    oclhcplus_hashModes.insert(2811, "IPB 2.0, MyBB 1.2");

    oclhclite_hashModes.insert(0, "MD5");
    oclhclite_hashModes.insert(10, "md5($pass.$salt)");
    oclhclite_hashModes.insert(11, "Joomla");
    oclhclite_hashModes.insert(100, "SHA1");
    oclhclite_hashModes.insert(101, "nsldap, SHA-1(Base64), Netscape LDAP SHA");
    oclhclite_hashModes.insert(110, "sha1($pass.$salt)");
    oclhclite_hashModes.insert(111, "nsldaps, SSHA-1(Base64), Netscape LDAP SSHA");
    oclhclite_hashModes.insert(112, "Oracle 11G");
    oclhclite_hashModes.insert(131, "MSSQL(2000)");
    oclhclite_hashModes.insert(132, "MSSQL(2005)");
    oclhclite_hashModes.insert(300, "MySQL");
    oclhclite_hashModes.insert(900, "MD4");
    oclhclite_hashModes.insert(1000, "NTLM");
    oclhclite_hashModes.insert(1100, "Domain Cached Credentials");
    oclhclite_hashModes.insert(1400, "SHA256");
    oclhclite_hashModes.insert(1500, "descrypt, DES(Unix), Traditional DES");
    oclhclite_hashModes.insert(1900, "SL3");
    oclhclite_hashModes.insert(2400, "Cisco-PIX MD5");
    oclhclite_hashModes.insert(2600, "md5(md5($pass))");
    oclhclite_hashModes.insert(2611, "vBulletin < v3.8.5");
    oclhclite_hashModes.insert(2711, "vBulletin > v3.8.5");
    oclhclite_hashModes.insert(2811, "IPB 2+, MyBB 1.2+");
    oclhclite_hashModes.insert(3000, "LM");


    this->add_hash_and_attack_modes(ui->comboBox_hc_attack, hc_attackModes);
    this->add_hash_and_attack_modes(ui->comboBox_oclhcplus_attack, oclhcplus_attackModes);

    this->add_hash_and_attack_modes(ui->comboBox_hc_hash, hc_hashModes);
    this->add_hash_and_attack_modes(ui->comboBox_oclhcplus_hash, oclhcplus_hashModes);
    this->add_hash_and_attack_modes(ui->comboBox_oclhclite_hash, oclhclite_hashModes);
}

void MainWindow::add_wordlist_item(QString &wordlist, qint16 hc_type) {
    QListWidget *w;
    bool duplicate = false;

    switch(hc_type) {
    case 0: w = ui->listWidget_hc_wordlist; break;
    case 1: w = ui->listWidget_oclhcplus_wordlist; break;
    default: return;
    }

    for (int j=0; j<w->count(); ++j) {
        if (w->item(j)->text() == wordlist) {
            duplicate = true;
        }
    }

    if (!wordlist.isNull() && !duplicate) {
        QListWidgetItem *newItem = new QListWidgetItem(QIcon(":/images/icon_dir.png"), wordlist, w);
        newItem->setCheckState(Qt::Checked);
        w->addItem(newItem);
    }

}

void MainWindow::add_wordlist_item(QStringList &wordlist, qint16 hc_type) {
    QListWidget *w;
    bool duplicate = false;

    switch(hc_type) {
    case 0: w = ui->listWidget_hc_wordlist; break;
    case 1: w = ui->listWidget_oclhcplus_wordlist; break;
    default: return;
    }

    for (int i=0; i<wordlist.length(); i++) {
        duplicate = false;
        for (int j=0; j<w->count(); ++j) {
            if (w->item(j)->text() == wordlist.at(i)) {
                duplicate = true;
            }
        }

        if (!wordlist.at(i).isNull() && !duplicate) {
            QListWidgetItem *newItem = new QListWidgetItem(QIcon(":/images/icon_file.png"), wordlist.at(i), w);
            newItem->setCheckState(Qt::Checked);
            w->addItem(newItem);
        }
    }
}

void MainWindow::set_outfile_path(qint16 hc_type) {
    QLineEdit *hash, *out;

    switch(hc_type) {
    case 0:
        hash = ui->lineEdit_hc_open_hashfile;
        out = ui->lineEdit_hc_outfile;
        break;
    case 1:
        hash = ui->lineEdit_oclhcplus_open_hashfile;
        out = ui->lineEdit_oclhcplus_outfile;
        break;
    case 2:
        hash = ui->lineEdit_oclhclite_hash;
        out = ui->lineEdit_oclhclite_outfile;
        break;
    default: return;
    }

    if (hash->text().length()) {
        out->setText(hash->text() + ".out");
    }
}

/**********************************************************/


/********* hashcat ****************************************/

void MainWindow::on_pushButton_hc_open_hashfile_clicked()
{
    QString hashfile = QFileDialog::getOpenFileName();
    if (!hashfile.isNull()) {
        ui->lineEdit_hc_open_hashfile->setText(QDir::toNativeSeparators(hashfile));
    }
}

void MainWindow::on_pushButton_hc_open_saltfile_clicked()
{
    QString saltfile = QFileDialog::getOpenFileName();
    if (!saltfile.isNull()) {
        ui->lineEdit_hc_open_saltfile->setText(QDir::toNativeSeparators(saltfile));
    }
}


void MainWindow::on_pushButton_hc_open_rulesfile_clicked()
{
    QString rulesfile = QFileDialog::getOpenFileName();
    if(!rulesfile.isNull()) {
        ui->lineEdit_hc_open_rulesfile->setText(QDir::toNativeSeparators(rulesfile));
    }
}

void MainWindow::on_pushButton_hc_open_tablefile_clicked()
{
    QString tablefile = QFileDialog::getOpenFileName();
    if(!tablefile.isNull()) {
        ui->lineEdit_hc_open_tablefile->setText(QDir::toNativeSeparators(tablefile));
    }
}

void MainWindow::on_toolButton_hc_wordlist_sort_asc_clicked()
{
    int currentRow = ui->listWidget_hc_wordlist->currentRow();
    if (currentRow == 0) return;
    QListWidgetItem *currentItem = ui->listWidget_hc_wordlist->takeItem(currentRow);
    ui->listWidget_hc_wordlist->insertItem(currentRow - 1, currentItem);
    ui->listWidget_hc_wordlist->setCurrentRow(currentRow - 1);
}

void MainWindow::on_toolButton_hc_wordlist_sort_desc_clicked()
{
    int currentRow = ui->listWidget_hc_wordlist->currentRow();
    if (currentRow >= ui->listWidget_hc_wordlist->count()-1) return;
    QListWidgetItem *currentItem = ui->listWidget_hc_wordlist->takeItem(currentRow);
    ui->listWidget_hc_wordlist->insertItem(currentRow + 1, currentItem);
    ui->listWidget_hc_wordlist->setCurrentRow(currentRow + 1);
}

void MainWindow::on_listWidget_hc_wordlist_itemClicked(QListWidgetItem* item)
{
    ui->pushButton_hc_remove_wordlist->setEnabled(true);
    ui->toolButton_hc_wordlist_sort_asc->setEnabled(true);
    ui->toolButton_hc_wordlist_sort_desc->setEnabled(true);
}

void MainWindow::on_pushButton_hc_add_wordlist_clicked()
{
    QStringList wordlist = QFileDialog::getOpenFileNames();
    this->add_wordlist_item(wordlist, 0);
}

void MainWindow::on_pushButton_hc_add_wordlist_folder_clicked()
{
    QString wordlist = QFileDialog::getExistingDirectory();
    this->add_wordlist_item(wordlist, 0);
}

void MainWindow::on_pushButton_hc_remove_wordlist_clicked()
{
    qDeleteAll(ui->listWidget_hc_wordlist->selectedItems());
    ui->pushButton_hc_remove_wordlist->setEnabled(false);
    ui->toolButton_hc_wordlist_sort_asc->setEnabled(false);
    ui->toolButton_hc_wordlist_sort_desc->setEnabled(false);
    ui->listWidget_hc_wordlist->clearSelection();
}

void MainWindow::on_checkBox_hc_custom1_toggled(bool checked)
{
    ui->lineEdit_hc_custom1->setEnabled(checked);
}

void MainWindow::on_checkBox_hc_custom2_toggled(bool checked)
{
    ui->lineEdit_hc_custom2->setEnabled(checked);
}

void MainWindow::on_checkBox_hc_custom3_toggled(bool checked)
{
    ui->lineEdit_hc_custom3->setEnabled(checked);
}

void MainWindow::on_checkBox_hc_custom4_toggled(bool checked)
{
    ui->lineEdit_hc_custom4->setEnabled(checked);
}

void MainWindow::on_pushButton_hc_output_clicked()
{
    QString outfile = QFileDialog::getSaveFileName();
    if (!outfile.isNull()) {
        ui->lineEdit_hc_outfile->setText(QDir::toNativeSeparators(outfile));
    }
}

void MainWindow::on_comboBox_hc_attack_currentIndexChanged(int index)
{
    hc_update_view_attack_mode();
}

void MainWindow::hc_update_view_attack_mode()
{
    int attackMode = hc_attackModes.key(ui->comboBox_hc_attack->currentText());
    bool groupWordlists = false,
         groupRules = false,
         groupToggle = false,
         groupPassword = false,
         groupBruteForce = false,
         groupTableLookup = false;

    switch(attackMode) {
    case 0:
    case 1:
        // Straight-Words + Combination-Words
        groupWordlists = true;
        groupRules = true;
        groupToggle = false;
        groupPassword = false;
        groupBruteForce = false;
        groupTableLookup = false;
        break;
    case 2:
        // Toggle-Case
        groupWordlists = true;
        groupRules = false;
        groupToggle = true;
        groupPassword = false;
        groupBruteForce = false;
        groupTableLookup = false;
        break;
    case 3:
        // Brute-Force
        groupWordlists = false;
        groupRules = false;
        groupToggle = false;
        groupPassword = false;
        groupBruteForce = true;
        groupTableLookup = false;
        break;
    case 4:
        // Permutation
        groupWordlists = true;
        groupRules = false;
        groupToggle = false;
        groupPassword = true;
        groupBruteForce = false;
        groupTableLookup = false;
        break;
    case 5:
        // Table-Lookup
        groupWordlists = true;
        groupRules = false;
        groupToggle = false;
        groupPassword = true;
        groupBruteForce = false;
        groupTableLookup = true;
        break;
    }

    ui->groupBox_hc_wordlists->setDisabled(!groupWordlists);
    ui->groupBox_hc_rules->setDisabled(!groupRules);
    ui->groupBox_hc_toggle->setDisabled(!groupToggle);
    ui->groupBox_hc_password->setDisabled(!groupPassword);
    ui->groupBox_hc_bruteforce->setDisabled(!groupBruteForce);
    ui->groupBox_hc_tablelookup->setDisabled(!groupTableLookup);
    this->hcCommandChanged();
}

void MainWindow::on_comboBox_hc_hash_currentIndexChanged(QString currentText)
{
    if (currentText.contains("$salt")) {
        ui->groupBox_hc_saltfile->setVisible(true);
    } else {
        ui->groupBox_hc_saltfile->setVisible(false);
    }
}

void MainWindow::on_checkBox_hc_saltfile_toggled(bool checked)
{
    if (checked) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("If you enable this option please make sure that you really need a salt file. In most cases this option is not necessary, even if your hashes are salted.");
        msgBox.exec();

    }
    ui->lineEdit_hc_open_saltfile->setEnabled(checked);
    ui->pushButton_hc_open_saltfile->setEnabled(checked);
}


void MainWindow::on_checkBox_hc_outfile_toggled(bool checked)
{
   ui->lineEdit_hc_outfile->setEnabled(checked);
   ui->pushButton_hc_output->setEnabled(checked);
}

void MainWindow::on_radioButton_hc_open_rulesfile_toggled(bool checked)
{
    ui->lineEdit_hc_open_rulesfile->setEnabled(checked);
    ui->pushButton_hc_open_rulesfile->setEnabled(checked);
}

void MainWindow::on_radioButton_hc_rules_generated_toggled(bool checked)
{
    ui->spinBox_hc_rules->setEnabled(checked);
}

void MainWindow::on_lineEdit_hc_open_hashfile_textChanged(const QString &arg1)
{
    this->set_outfile_path(0);
}

QStringList MainWindow::hc_generate_arguments()
{
    QStringList arguments;
    int attackMode = hc_attackModes.key(ui->comboBox_hc_attack->currentText());

    if(hc_hashModes.key(ui->comboBox_hc_hash->currentText()) != 0) {
        arguments << "--hash-mode" << QString::number(hc_hashModes.key(ui->comboBox_hc_hash->currentText()));
    }

    if (attackMode != 0) {
        arguments << "--attack-mode" << QString::number(attackMode);
    }

    if (ui->checkBox_hc_remove->isChecked()) {
        arguments << "--remove";
    }

    if (ui->checkBox_hc_potfile->isChecked()) {
        arguments << "--disable-potfile";
    }

    if (ui->checkBox_hc_outfile->isChecked() && !ui->lineEdit_hc_outfile->text().isEmpty()) {
        QFileInfo hash_fi(ui->lineEdit_hc_open_hashfile->text());
        QString outfile = ui->lineEdit_hc_outfile->text();
        outfile.replace("<unixtime>", QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));
        outfile.replace("<hash>", hash_fi.fileName(), Qt::CaseInsensitive);
        arguments << "--output-file" << outfile;
    }

    if (ui->comboBox_hc_outfile_format->currentIndex() != 0) {
        arguments << "--output-format" << QString::number(ui->comboBox_hc_outfile_format->currentIndex());
    }

    if (!ui->lineEdit_hc_separator->text().isEmpty() && ui->lineEdit_hc_separator->text() != ":") {
        arguments << "--seperator-char" << ui->lineEdit_hc_separator->text();
    }

    if (ui->groupBox_hc_saltfile->isVisible() && ui->checkBox_hc_saltfile->isChecked() && !ui->lineEdit_hc_open_saltfile->text().isEmpty()) {
        arguments << "--salt-file" << ui->lineEdit_hc_open_saltfile->text();
    }

    switch(attackMode) {
    case 0:
    case 1:
        if (ui->radioButton_hc_open_rulesfile->isChecked() && !ui->lineEdit_hc_open_rulesfile->text().isEmpty()) {
            arguments << "--rules-file" << ui->lineEdit_hc_open_rulesfile->text();
        } else if (ui->radioButton_hc_rules_generated->isChecked()) {
            arguments << "--generate-rules" << ui->spinBox_hc_rules->cleanText();
        }
        break;
    case 2:
        arguments << "--toggle-min" << ui->spinBox_hc_toggle_min->cleanText();
        arguments << "--toggle-max" << ui->spinBox_hc_toggle_max->cleanText();
        break;
    case 3:
        if (ui->groupBox_hc_custom_charset->isEnabled()) {
            if (ui->checkBox_hc_custom1->isChecked() && !ui->lineEdit_hc_custom1->text().isEmpty()) {
                arguments << "--custom-charset1" << ui->lineEdit_hc_custom1->text();
            }

            if (ui->checkBox_hc_custom2->isChecked() && !ui->lineEdit_hc_custom2->text().isEmpty()) {
                arguments << "--custom-charset2" << ui->lineEdit_hc_custom2->text();
            }

            if (ui->checkBox_hc_custom3->isChecked() && !ui->lineEdit_hc_custom3->text().isEmpty()) {
                arguments << "--custom-charset3" << ui->lineEdit_hc_custom3->text();
            }

            if (ui->checkBox_hc_custom4->isChecked() && !ui->lineEdit_hc_custom4->text().isEmpty()) {
                arguments << "--custom-charset4" << ui->lineEdit_hc_custom4->text();
            }
        }
        break;
    case 4:
        arguments << "--perm-min" << ui->spinBox_hc_password_min->cleanText();
        arguments << "--perm-max" << ui->spinBox_hc_password_max->cleanText();
        break;
    case 5:
        arguments << "--table-min" << ui->spinBox_hc_password_min->cleanText();
        arguments << "--table-max" << ui->spinBox_hc_password_max->cleanText();
        if (!ui->lineEdit_hc_open_tablefile->text().isEmpty()) {
            arguments << "--table-file" << ui->lineEdit_hc_open_tablefile->text();
        }
        break;
    }

    if (!ui->spinBox_hc_threads->cleanText().isEmpty() && ui->spinBox_hc_threads->cleanText() != "8") {
        arguments << "--threads" << ui->spinBox_hc_threads->cleanText();
    }

    if (!ui->spinBox_hc_segment->cleanText().isEmpty() && ui->spinBox_hc_segment->cleanText() != "32") {
        arguments << "--segment-size" << ui->spinBox_hc_segment->cleanText();
    }

    if (!ui->spinBox_hc_skip->cleanText().isEmpty() && ui->spinBox_hc_skip->cleanText() != "0") {
        arguments << "--words-skip" << ui->spinBox_hc_skip->cleanText();
    }

    if (!ui->spinBox_hc_limit->cleanText().isEmpty() && ui->spinBox_hc_limit->cleanText() != "0") {
        arguments << "--words-limit" << ui->spinBox_hc_limit->cleanText();
    }

    if (!ui->lineEdit_hc_open_hashfile->text().isEmpty()) {
        arguments << ui->lineEdit_hc_open_hashfile->text();
    }

    if (ui->groupBox_hc_wordlists->isEnabled()) {
        for(int i=0; i<ui->listWidget_hc_wordlist->count(); i++) {
            if(ui->listWidget_hc_wordlist->item(i)->checkState() == Qt::Checked) {
                arguments << ui->listWidget_hc_wordlist->item(i)->text();
            }
        }
    } else {
        arguments << ui->lineEdit_hc_mask->text();
    }

    return arguments;

}

void MainWindow::on_pushButton_hc_execute_clicked()
{
    QStringList env = generate_terminal_env(0);
    QStringList arguments = hc_generate_arguments();
    hcCommandChanged(arguments.join(" "));

    if (ui->lineEdit_hc_open_hashfile->text().isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Please choose a hash file.");
        msgBox.exec();
        return;
    }

    QProcess proc;
    proc.startDetached(envInfo.value("terminal"), env << arguments, envInfo.value("dir_hc"));

}

/**********************************************************/

/********* oclHashcat-plus / cudaHashcat-plus *********************/

void MainWindow::on_comboBox_oclhcplus_attack_currentIndexChanged(int index)
{
    oclhcplus_update_view_attack_mode();
}

void MainWindow::oclhcplus_update_view_attack_mode()
{
    int attackMode = oclhcplus_attackModes.key(ui->comboBox_oclhcplus_attack->currentText());
    bool groupWordlists = false, groupRules = false, groupPassword = false, groupBruteForce = false, groupMask = false;

    switch(attackMode) {
    case 0:
        // Straight
        groupWordlists = true;
        groupRules = true;
        groupPassword = false;
        groupBruteForce = false;
        groupMask = false;
        break;
    case 1:
        // Combination
        groupWordlists = true;
        groupRules = false;
        groupPassword = false;
        groupBruteForce = false;
        groupMask = false;
        break;
    case 3:
        // Brute-force
        groupWordlists = false;
        groupRules = false;
        groupPassword = false;
        groupBruteForce = true;
        groupMask = true;
        break;
    case 4:
        // Permutation
        groupWordlists = true;
        groupRules = false;
        groupPassword = true;
        groupBruteForce = false;
        groupMask = false;
        break;
    case 6:
    case 7:
        // Hybrid dict + mask
        groupWordlists = true;
        groupRules = false;
        groupPassword = false;
        groupBruteForce = false;
        groupMask = true;
        break;
    }

    ui->groupBox_oclhcplus_wordlists->setDisabled(!groupWordlists);
    ui->groupBox_oclhcplus_rules->setDisabled(!groupRules);
    ui->groupBox_oclhcplus_password->setDisabled(!groupPassword);
    ui->groupBox_oclhcplus_custom_charset->setDisabled(!groupMask);
    ui->groupBox_oclhcplus_mask->setDisabled(!groupMask);
    this->oclhcplusCommandChanged();
}

void MainWindow::on_pushButton_oclhcplus_open_hashfile_clicked()
{
    QString hashfile = QFileDialog::getOpenFileName();
    if (!hashfile.isNull()) {
        ui->lineEdit_oclhcplus_open_hashfile->setText(QDir::toNativeSeparators(hashfile));
    }
}

void MainWindow::on_pushButton_oclhcplus_output_clicked()
{
    QString outfile = QFileDialog::getSaveFileName();
    if (!outfile.isNull()) {
        ui->lineEdit_oclhcplus_outfile->setText(QDir::toNativeSeparators(outfile));
    }
}

void MainWindow::on_pushButton_oclhcplus_remove_wordlist_clicked()
{
    qDeleteAll(ui->listWidget_oclhcplus_wordlist->selectedItems());
    ui->pushButton_oclhcplus_remove_wordlist->setEnabled(false);
    ui->toolButton_oclhcplus_wordlist_sort_asc->setEnabled(false);
    ui->toolButton_oclhcplus_wordlist_sort_desc->setEnabled(false);
    ui->listWidget_oclhcplus_wordlist->clearSelection();
}

void MainWindow::on_pushButton_oclhcplus_add_wordlist_clicked()
{
    QStringList wordlist = QFileDialog::getOpenFileNames();
    this->add_wordlist_item(wordlist, 1);
}

void MainWindow::on_pushButton_oclhcplus_add_wordlist_folder_clicked()
{
    QString wordlist = QFileDialog::getExistingDirectory();
    this->add_wordlist_item(wordlist, 1);
}

void MainWindow::on_toolButton_oclhcplus_wordlist_sort_asc_clicked()
{
    int currentRow = ui->listWidget_oclhcplus_wordlist->currentRow();
    if (currentRow == 0) return;
    QListWidgetItem *currentItem = ui->listWidget_oclhcplus_wordlist->takeItem(currentRow);
    ui->listWidget_oclhcplus_wordlist->insertItem(currentRow - 1, currentItem);
    ui->listWidget_oclhcplus_wordlist->setCurrentRow(currentRow - 1);
}

void MainWindow::on_toolButton_oclhcplus_wordlist_sort_desc_clicked()
{
    int currentRow = ui->listWidget_oclhcplus_wordlist->currentRow();
    if (currentRow >= ui->listWidget_oclhcplus_wordlist->count()-1) return;
    QListWidgetItem *currentItem = ui->listWidget_oclhcplus_wordlist->takeItem(currentRow);
    ui->listWidget_oclhcplus_wordlist->insertItem(currentRow + 1, currentItem);
    ui->listWidget_oclhcplus_wordlist->setCurrentRow(currentRow + 1);
}

void MainWindow::on_listWidget_oclhcplus_wordlist_itemClicked(QListWidgetItem* item)
{
    ui->pushButton_oclhcplus_remove_wordlist->setEnabled(true);
    ui->toolButton_oclhcplus_wordlist_sort_asc->setEnabled(true);
    ui->toolButton_oclhcplus_wordlist_sort_desc->setEnabled(true);
}

void MainWindow::on_checkBox_oclhcplus_rulesfile_1_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_open_rulesfile_1->setEnabled(checked);
    ui->pushButton_oclhcplus_open_rulesfile_1->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_rulesfile_2_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_open_rulesfile_2->setEnabled(checked);
    ui->pushButton_oclhcplus_open_rulesfile_2->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_rulesfile_3_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_open_rulesfile_3->setEnabled(checked);
    ui->pushButton_oclhcplus_open_rulesfile_3->setEnabled(checked);
}

void MainWindow::on_radioButton_oclhcplus_use_rules_file_toggled(bool checked)
{
    ui->checkBox_oclhcplus_rulesfile_1->setEnabled(checked);
    ui->checkBox_oclhcplus_rulesfile_2->setEnabled(checked);
    ui->checkBox_oclhcplus_rulesfile_3->setEnabled(checked);
    if (checked) {
        this->on_checkBox_oclhcplus_rulesfile_1_toggled(ui->checkBox_oclhcplus_rulesfile_1->isChecked());
        this->on_checkBox_oclhcplus_rulesfile_2_toggled(ui->checkBox_oclhcplus_rulesfile_2->isChecked());
        this->on_checkBox_oclhcplus_rulesfile_3_toggled(ui->checkBox_oclhcplus_rulesfile_3->isChecked());
    }
}

void MainWindow::on_radioButton_oclhcplus_generate_rules_toggled(bool checked)
{
    ui->spinBox_oclhcplus_generate_rules->setEnabled(checked);
    if (checked) {
        this->on_radioButton_oclhcplus_use_rules_file_toggled(false);
        this->on_checkBox_oclhcplus_rulesfile_1_toggled(false);
        this->on_checkBox_oclhcplus_rulesfile_2_toggled(false);
        this->on_checkBox_oclhcplus_rulesfile_3_toggled(false);
    }
}

void MainWindow::on_pushButton_oclhcplus_open_rulesfile_1_clicked()
{
    QString rulesfile = QFileDialog::getOpenFileName();
    if (!rulesfile.isNull()) {
        ui->lineEdit_oclhcplus_open_rulesfile_1->setText(QDir::toNativeSeparators(rulesfile));
    }
}

void MainWindow::on_pushButton_oclhcplus_open_rulesfile_2_clicked()
{
    QString rulesfile = QFileDialog::getOpenFileName();
    if (!rulesfile.isNull()) {
        ui->lineEdit_oclhcplus_open_rulesfile_2->setText(QDir::toNativeSeparators(rulesfile));
    }
}

void MainWindow::on_pushButton_oclhcplus_open_rulesfile_3_clicked()
{
    QString rulesfile = QFileDialog::getOpenFileName();
    if (!rulesfile.isNull()) {
        ui->lineEdit_oclhcplus_open_rulesfile_3->setText(QDir::toNativeSeparators(rulesfile));
    }
}

void MainWindow::on_checkBox_oclhcplus_custom1_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_custom1->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_custom2_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_custom2->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_custom3_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_custom3->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_custom4_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_custom4->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhcplus_outfile_toggled(bool checked)
{
    ui->lineEdit_oclhcplus_outfile->setEnabled(checked);
    ui->pushButton_oclhcplus_output->setEnabled(checked);
}

void MainWindow::on_lineEdit_oclhcplus_open_hashfile_textChanged(const QString &arg1)
{
    this->set_outfile_path(1);
}

QStringList MainWindow::oclhcplus_generate_arguments()
{
    QStringList arguments;
    QString mask_before_dict = "";
    QString mask_after_dict = "";

    int attackMode = oclhcplus_attackModes.key(ui->comboBox_oclhcplus_attack->currentText());

    if(oclhcplus_hashModes.key(ui->comboBox_oclhcplus_hash->currentText()) != 0) {
        arguments << "--hash-type" << QString::number(oclhcplus_hashModes.key(ui->comboBox_oclhcplus_hash->currentText()));
    }

    if ( attackMode != 0 ) {
        arguments << "--attack-mode" << QString::number(attackMode);
    }

    if (ui->checkBox_oclhcplus_remove->isChecked()) {
        arguments << "--remove";
    }

    if (ui->checkBox_oclhcplus_ignoreusername->isChecked()) {
        arguments << "--username";
    }

    switch(attackMode) {
    case 0:
        if ( ui->radioButton_oclhcplus_use_rules_file->isChecked()) {
            if (ui->checkBox_oclhcplus_rulesfile_1->isChecked() && !ui->lineEdit_oclhcplus_open_rulesfile_1->text().isEmpty()) {
                arguments << "--rules-file" << ui->lineEdit_oclhcplus_open_rulesfile_1->text();
            }

            if (ui->checkBox_oclhcplus_rulesfile_2->isChecked() && !ui->lineEdit_oclhcplus_open_rulesfile_2->text().isEmpty()) {
                arguments << "--rules-file" << ui->lineEdit_oclhcplus_open_rulesfile_2->text();
            }

            if (ui->checkBox_oclhcplus_rulesfile_3->isChecked() && !ui->lineEdit_oclhcplus_open_rulesfile_3->text().isEmpty()) {
                arguments << "--rules-file" << ui->lineEdit_oclhcplus_open_rulesfile_3->text();
            }
        } else if (ui->radioButton_oclhcplus_generate_rules->isChecked() && !ui->spinBox_oclhcplus_generate_rules->cleanText().isEmpty()) {
            arguments << "--generate-rules" << ui->spinBox_oclhcplus_generate_rules->cleanText();
        }
        break;
    case 1:
        break;
    case 3:
        mask_before_dict = ui->lineEdit_oclhcplus_mask->text();
        break;
    case 4:
        arguments << "--perm-min" << ui->spinBox_oclhcplus_password_min->cleanText();
        arguments << "--perm-max" << ui->spinBox_oclhcplus_password_max->cleanText();
        break;
    case 6:
        if (!ui->lineEdit_oclhcplus_mask->text().isEmpty()) {
            mask_after_dict = ui->lineEdit_oclhcplus_mask->text();
        }
        break;
    case 7:
        if (!ui->lineEdit_oclhcplus_mask->text().isEmpty()) {
            mask_before_dict = ui->lineEdit_oclhcplus_mask->text();
        }
        break;
    }

    if (ui->groupBox_oclhcplus_custom_charset->isEnabled()) {
        if (ui->checkBox_oclhcplus_custom1->isChecked() && !ui->lineEdit_oclhcplus_custom1->text().isEmpty()) {
            arguments << "--custom-charset1" << ui->lineEdit_oclhcplus_custom1->text();
        }

        if (ui->checkBox_oclhcplus_custom2->isChecked() && !ui->lineEdit_oclhcplus_custom2->text().isEmpty()) {
            arguments << "--custom-charset2" << ui->lineEdit_oclhcplus_custom2->text();
        }

        if (ui->checkBox_oclhcplus_custom3->isChecked() && !ui->lineEdit_oclhcplus_custom3->text().isEmpty()) {
            arguments << "--custom-charset3" << ui->lineEdit_oclhcplus_custom3->text();
        }

        if (ui->checkBox_oclhcplus_custom4->isChecked() && !ui->lineEdit_oclhcplus_custom4->text().isEmpty()) {
            arguments << "--custom-charset4" << ui->lineEdit_oclhcplus_custom4->text();
        }
    }

    if (ui->checkBox_oclhcplus_hex_hash->isChecked()) {
        arguments << "--hex-charset";
    }

    if (ui->checkBox_oclhcplus_hex_salt->isChecked()) {
        arguments << "--hex-salt";
    }

    if(ui->checkBox_oclhcplus_outfile->isChecked() && !ui->lineEdit_oclhcplus_outfile->text().isEmpty()) {
        QFileInfo hash_fi(ui->lineEdit_oclhcplus_open_hashfile->text());
        QString outfile = ui->lineEdit_oclhcplus_outfile->text();
        outfile.replace("<unixtime>", QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));
        outfile.replace("<hash>", hash_fi.fileName(), Qt::CaseInsensitive);
        arguments << "--outfile" << outfile;
    }

    if (ui->comboBox_oclhcplus_outfile_format->currentIndex() != 2) {
        arguments << "--outfile-format" << QString::number(ui->comboBox_oclhcplus_outfile_format->currentIndex()+1);
    }

    if (ui->checkBox_oclhcplus_async->isChecked()) {
        arguments << "--gpu-async";
    }

    if (!ui->lineEdit_oclhcplus_cpu_affinity->text().isEmpty()) {
        arguments << "--cpu-affinity" << ui->lineEdit_oclhcplus_cpu_affinity->text();
    }

    if (!ui->lineEdit_oclhcplus_devices->text().isEmpty() && ui->lineEdit_oclhcplus_devices->text() != "0") {
        arguments << "--gpu-devices" << ui->lineEdit_oclhcplus_devices->text();
    }

    if (!ui->spinBox_oclhcplus_accel->cleanText().isEmpty() && ui->spinBox_oclhcplus_accel->cleanText() != "8") {
        arguments << "--gpu-accel" << ui->spinBox_oclhcplus_accel->cleanText();
    }

    if (!ui->spinBox_oclhcplus_loops->cleanText().isEmpty() && ui->spinBox_oclhcplus_loops->cleanText() != "256") {
        arguments << "--gpu-loops" << ui->spinBox_oclhcplus_loops->cleanText();
    }

    if (!ui->spinBox_oclhcplus_watchdog->cleanText().isEmpty() && ui->spinBox_oclhcplus_watchdog->cleanText() != "90") {
        arguments << "--gpu-watchdog" << ui->spinBox_oclhcplus_watchdog->cleanText();
    }

    if (!ui->spinBox_oclhcplus_segment->cleanText().isEmpty() && ui->spinBox_oclhcplus_segment->cleanText() != "32") {
        arguments << "--segment-size" << ui->spinBox_oclhcplus_segment->cleanText();
    }

    if (!ui->lineEdit_oclhcplus_open_hashfile->text().isEmpty()) {
        arguments << ui->lineEdit_oclhcplus_open_hashfile->text();
    }

    if (mask_before_dict.length()) {
        arguments << mask_before_dict;
    }

    if (ui->groupBox_oclhcplus_wordlists->isEnabled()) {
        for(int i=0; i<ui->listWidget_oclhcplus_wordlist->count(); i++) {
            if(ui->listWidget_oclhcplus_wordlist->item(i)->checkState() == Qt::Checked) {
                arguments << ui->listWidget_oclhcplus_wordlist->item(i)->text();
            }
        }
    }

    if (mask_after_dict.length()) {
        arguments << mask_after_dict;
    }

    return arguments;
}

void MainWindow::on_pushButton_oclhcplus_execute_clicked()
{
    QStringList env = generate_terminal_env(1);
    QStringList arguments = oclhcplus_generate_arguments();
    oclhcplusCommandChanged(arguments.join(" "));

    if (ui->lineEdit_oclhcplus_open_hashfile->text().isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Please choose a hash file.");
        msgBox.exec();
        return;
    }

    QProcess proc;
    proc.startDetached(envInfo.value("terminal"), env << arguments, envInfo.value("dir_oclhcplus"));
}



/**********************************************************/

/********* oclHashcat-lite / cudaHashcat-lite *************/

void MainWindow::on_pushButton_oclhclite_output_clicked()
{
    QString outfile = QFileDialog::getSaveFileName();
    if (!outfile.isNull()) {
        ui->lineEdit_oclhclite_outfile->setText(QDir::toNativeSeparators(outfile));
    }
}

void MainWindow::on_checkBox_oclhclite_outfile_toggled(bool checked)
{
    ui->lineEdit_oclhclite_outfile->setEnabled(checked);
    ui->pushButton_oclhclite_output->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_mask_toggled(bool checked)
{
    ui->lineEdit_oclhclite_mask->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_custom1_toggled(bool checked)
{
    ui->lineEdit_oclhclite_custom1->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_custom2_toggled(bool checked)
{
    ui->lineEdit_oclhclite_custom2->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_custom3_toggled(bool checked)
{
    ui->lineEdit_oclhclite_custom3->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_custom4_toggled(bool checked)
{
    ui->lineEdit_oclhclite_custom4->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_skip_toggled(bool checked)
{
  ui->lineEdit_oclhclite_skip->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_limit_toggled(bool checked)
{
  ui->lineEdit_oclhclite_limit->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_skip_plain_toggled(bool checked)
{
    ui->lineEdit_oclhclite_skip_plain->setEnabled(checked);
}

void MainWindow::on_checkBox_oclhclite_limit_plain_toggled(bool checked)
{
    ui->lineEdit_oclhclite_limit_plain->setEnabled(checked);
}

void MainWindow::on_lineEdit_oclhclite_hash_textChanged(const QString &arg1)
{
    this->set_outfile_path(2);
}

QStringList MainWindow::oclhclite_generate_arguments()
{
    QStringList arguments;

    if(oclhclite_hashModes.key(ui->comboBox_oclhclite_hash->currentText()) != 0) {
        arguments << "--hash-type" << QString::number(oclhclite_hashModes.key(ui->comboBox_oclhclite_hash->currentText()));
    }

    if (!ui->lineEdit_oclhclite_separator->text().isEmpty() && ui->lineEdit_oclhclite_separator->text() != ":") {
        arguments << "--seperator-char" << ui->lineEdit_oclhclite_separator->text();
    }

    if (ui->checkBox_oclhclite_outfile->isChecked() && !ui->lineEdit_oclhclite_outfile->text().isEmpty()) {
        QFileInfo hash_fi(ui->lineEdit_oclhclite_hash->text());
        QString outfile = ui->lineEdit_oclhclite_outfile->text();
        outfile.replace("<unixtime>", QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));
        outfile.replace("<hash>", hash_fi.fileName(), Qt::CaseInsensitive);
        arguments << "--outfile" << outfile;
    }

    if (ui->comboBox_oclhclite_outfile_format->currentIndex() != 2) {
        arguments << "--outfile-format" << QString::number(ui->comboBox_oclhclite_outfile_format->currentIndex()+1);
    }

    if (ui->checkBox_oclhclite_custom1->isChecked() && !ui->lineEdit_oclhclite_custom1->text().isEmpty()) {
        arguments << "--custom-charset1" << ui->lineEdit_oclhclite_custom1->text();
    }

    if (ui->checkBox_oclhclite_custom2->isChecked() && !ui->lineEdit_oclhclite_custom2->text().isEmpty()) {
        arguments << "--custom-charset2" << ui->lineEdit_oclhclite_custom2->text();
    }

    if (ui->checkBox_oclhclite_custom3->isChecked() && !ui->lineEdit_oclhclite_custom3->text().isEmpty()) {
        arguments << "--custom-charset3" << ui->lineEdit_oclhclite_custom3->text();
    }

    if (ui->checkBox_oclhclite_custom4->isChecked() && !ui->lineEdit_oclhclite_custom4->text().isEmpty()) {
        arguments << "--custom-charset4" << ui->lineEdit_oclhclite_custom4->text();
    }

    if (ui->spinBox_oclhclite_password_min->text() != "1") {
        arguments << "--pw-min" << ui->spinBox_oclhclite_password_min->cleanText();
    }

    if (ui->spinBox_oclhclite_password_max->text() != "15") {
        arguments << "--pw-max" << ui->spinBox_oclhclite_password_max->cleanText();
    }

    if (ui->checkBox_oclhclite_hex_hash->isChecked()) {
        arguments << "--hex-charset";
    }

    if (ui->checkBox_oclhclite_hex_salt->isChecked()) {
        arguments << "--hex-salt";
    }

    if (ui->checkBox_oclhclite_skip->isChecked() && !ui->lineEdit_oclhclite_skip->text().isEmpty()) {
        arguments << "--pw-skip" << ui->lineEdit_oclhclite_skip->text();
    }

    if (ui->checkBox_oclhclite_limit->isChecked() && !ui->lineEdit_oclhclite_limit->text().isEmpty()) {
        arguments << "--pw-limit" << ui->lineEdit_oclhclite_limit->text();
    }

    if (ui->checkBox_oclhclite_skip_plain->isChecked() && !ui->lineEdit_oclhclite_skip_plain->text().isEmpty()) {
        arguments << "--pw-skip-plain" << ui->lineEdit_oclhclite_skip_plain->text();
    }

    if (ui->checkBox_oclhclite_limit_plain->isChecked() && !ui->lineEdit_oclhclite_limit_plain->text().isEmpty()) {
        arguments << "--pw-limit-plain" << ui->lineEdit_oclhclite_limit_plain->text();
    }

    if (ui->checkBox_oclhclite_async->isChecked()) {
        arguments << "--gpu-async";
    }

    if (!ui->lineEdit_oclhclite_cpu_affinity->text().isEmpty()) {
        arguments << "--cpu-affinity" << ui->lineEdit_oclhclite_cpu_affinity->text();
    }

    if (!ui->lineEdit_oclhclite_devices->text().isEmpty() && ui->lineEdit_oclhclite_devices->text() != "0") {
        arguments << "--gpu-devices" << ui->lineEdit_oclhclite_devices->text();
    }

    if (!ui->spinBox_oclhclite_accel->cleanText().isEmpty() && ui->spinBox_oclhclite_accel->cleanText() != "8") {
        arguments << "--gpu-accel" << ui->spinBox_oclhclite_accel->cleanText();
    }

    if (!ui->spinBox_oclhclite_loops->cleanText().isEmpty() && ui->spinBox_oclhclite_loops->cleanText() != "256") {
        arguments << "--gpu-loops" << ui->spinBox_oclhclite_loops->cleanText();
    }

    if (!ui->spinBox_oclhclite_watchdog->cleanText().isEmpty() && ui->spinBox_oclhclite_watchdog->cleanText() != "90") {
        arguments << "--gpu-watchdog" << ui->spinBox_oclhclite_watchdog->cleanText();
    }

    if (!ui->lineEdit_oclhclite_hash->text().isEmpty()) {
        arguments << ui->lineEdit_oclhclite_hash->text();
    }

    if (ui->checkBox_oclhclite_mask->isChecked() && !ui->lineEdit_oclhclite_mask->text().isEmpty()) {
        arguments << ui->lineEdit_oclhclite_mask->text();
    }

    return arguments;
}

void MainWindow::on_pushButton_oclhclite_execute_clicked()
{
    QStringList env = generate_terminal_env(2);
    QStringList arguments = oclhclite_generate_arguments();
    oclhcliteCommandChanged(arguments.join(" "));

    if (ui->lineEdit_oclhclite_hash->text().isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Please enter a hash.");
        msgBox.exec();
        return;
    }

    QProcess proc;
    proc.startDetached(envInfo.value("terminal"), env << arguments, envInfo.value("dir_oclhclite"));
}

/**********************************************************/

