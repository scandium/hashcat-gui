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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
    class aboutdialog;
}

class aboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit aboutDialog(QWidget *parent = 0);
    ~aboutDialog();
    void get_versions(QMap <QString, QString> &);

private slots:
    void on_pushButton_about_ok_clicked();

    void read_hc();

    void read_oclhcplus();

    void read_oclhclite();

    void finished_proc_hc();

    void finished_proc_oclhcplus();

    void finished_proc_oclhclite();

private:
    Ui::aboutdialog *ui;

    QProcess *proc_hc, *proc_oclhc, *proc_oclhcplus, *proc_oclhclite;
};

#endif // ABOUTDIALOG_H
