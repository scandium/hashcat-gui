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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QProcess>
#include <QMessageBox>

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutdialog)
{
    ui->setupUi(this);
    setFixedSize(width(), height());
}

aboutDialog::~aboutDialog()
{
    delete ui;
}

void aboutDialog::on_pushButton_about_ok_clicked()
{
    this->close();
}

void aboutDialog::get_versions(QMap <QString, QString> &info) {

#if defined(Q_WS_WIN)
    QString exec_hc = info.value("dir_hc") + info.value("cmd_hc");
    QString exec_oclhcplus = info.value("dir_oclhcplus") + info.value("cmd_oclhcplus");
    QString exec_oclhclite = info.value("dir_oclhclite") + info.value("cmd_oclhclite");
#else
    QString exec_hc = info.value("dir_current") + info.value("cmd_hc");
    QString exec_oclhcplus = info.value("dir_current") + info.value("cmd_oclhcplus");
    QString exec_oclhclite = info.value("dir_current") + info.value("cmd_oclhclite");
#endif


    if(info.value("cmd_hc").length() > 0) {
        proc_hc = new QProcess(this);
        proc_hc->setWorkingDirectory(info.value("dir_hc"));

        connect(proc_hc, SIGNAL(readyRead()), this, SLOT(read_hc()) );
        connect(proc_hc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished_proc_hc()) );

        proc_hc->start(exec_hc, QStringList() << "--version");
        ui->label_hc_version_text->setText(info.value("cmd_hc"));
    }

    if(info.value("cmd_oclhcplus").length() > 0) {
        proc_oclhcplus = new QProcess(this);
        proc_oclhcplus->setWorkingDirectory(info.value("dir_oclhcplus"));

        connect(proc_oclhcplus, SIGNAL(readyRead()), this, SLOT(read_oclhcplus()) );
        connect(proc_oclhcplus, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished_proc_oclhcplus()) );

        proc_oclhcplus->start(exec_oclhcplus, QStringList() << "--version");
        ui->label_oclhcplus_version_text->setText(info.value("cmd_oclhcplus"));
    }

    if(info.value("cmd_oclhclite").length() > 0) {
        proc_oclhclite = new QProcess(this);
        proc_oclhclite->setWorkingDirectory(info.value("dir_oclhclite"));

        connect(proc_oclhclite, SIGNAL(readyRead()), this, SLOT(read_oclhclite()) );
        connect(proc_oclhclite, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished_proc_oclhclite()) );

        proc_oclhclite->start(exec_oclhclite, QStringList() << "--version");
        ui->label_oclhclite_version_text->setText(info.value("cmd_oclhclite"));
    }
}

void aboutDialog::read_hc()
{
    while (proc_hc->canReadLine())
        ui->label_hc_version->setText(proc_hc->readLine().replace("\n", ""));
}

void aboutDialog::read_oclhcplus()
{
    while (proc_oclhcplus->canReadLine())
        ui->label_oclhcplus_version->setText(proc_oclhcplus->readLine().replace("\n", ""));
}

void aboutDialog::read_oclhclite()
{
    while (proc_oclhclite->canReadLine())
        ui->label_oclhclite_version->setText(proc_oclhclite->readLine().replace("\n", ""));
}

void aboutDialog::finished_proc_hc() {
    this->proc_hc = NULL;
}

void aboutDialog::finished_proc_oclhcplus() {
    this->proc_oclhcplus = NULL;
}

void aboutDialog::finished_proc_oclhclite() {
    this->proc_oclhclite = NULL;
}
