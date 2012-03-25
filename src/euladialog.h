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

#ifndef EULADIALOG_H
#define EULADIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
    class eulaDialog;
}

class eulaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit eulaDialog(QWidget *parent = 0);
    ~eulaDialog();
    void get_eula(QMap <QString, QString> &, quint16);

private slots:

    void on_pushButton_eula_ok_clicked();

    void readFromStdout();

    void finished_proc();

    void proc_error(QProcess::ProcessError);

private:
    Ui::eulaDialog *ui;

    QProcess *proc;
};

#endif // EULADIALOG_H
