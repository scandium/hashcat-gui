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

#include "euladialog.h"
#include "ui_euladialog.h"
#include <QProcess>

eulaDialog::eulaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::eulaDialog)
{
    ui->setupUi(this);
    setFixedSize(width(), height());
}

eulaDialog::~eulaDialog()
{
    delete ui;
}

void eulaDialog::on_pushButton_eula_ok_clicked()
{
    this->close();
}

void eulaDialog::get_eula(QMap <QString, QString> &info, quint16 hc_type) {

    QString exec, workDir;

#if defined(Q_WS_WIN)
    switch(hc_type) {
    case 0: exec = info.value("dir_hc") + info.value("cmd_hc"); workDir = info.value("dir_hc"); break;
    case 1: exec = info.value("dir_oclhcplus") + info.value("cmd_oclhcplus"); workDir = info.value("dir_oclhcplus"); break;
    case 2: exec = info.value("dir_oclhclite") + info.value("cmd_oclhclite"); workDir = info.value("dir_oclhclite"); break;
    }
#else
    switch(hc_type) {
    case 0: exec = info.value("dir_current") + info.value("cmd_hc"); workDir = info.value("dir_hc"); break;
    case 1: exec = info.value("dir_current") + info.value("cmd_oclhcplus"); workDir = info.value("dir_oclhcplus"); break;
    case 2: exec = info.value("dir_current") + info.value("cmd_oclhclite"); workDir = info.value("dir_oclhclite"); break;
    }
#endif

    ui->textEdit_eula_text->clear();
    proc = new QProcess(this);
    proc->setWorkingDirectory(workDir);
    connect(proc, SIGNAL(readyRead()), this, SLOT(readFromStdout()) );
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished_proc()) );
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(proc_error(QProcess::ProcessError)) );
    proc->start(exec, QStringList() << "--eula");
}

void eulaDialog::readFromStdout()
{
    while (proc->canReadLine())
        ui->textEdit_eula_text->insertPlainText(proc->readLine());
}

void eulaDialog::finished_proc() {
    this->proc = NULL;
}

void eulaDialog::proc_error(QProcess::ProcessError error) {
    if (ui->textEdit_eula_text->toPlainText().isEmpty()) {
        ui->textEdit_eula_text->insertPlainText("Could not read EULA: Error code " + QString::number(error));
    }
    finished_proc();
}
