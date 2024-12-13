#ifndef CALCTIMESTAMPDLG_H
#define CALCTIMESTAMPDLG_H

#include <QDialog>

namespace Ui {
class CalcTimestampDlg;
}

class CalcTimestampDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CalcTimestampDlg(QWidget *parent = nullptr);
    ~CalcTimestampDlg();

    QString getTimestampString();

    void setTimestamp(qint64 stamp);

private:
    qint64 getUnixTimestampFromUTCTime(const QString& timeString);

private slots:
    void on_buttonConvert_clicked();

    void on_buttonConvert2_clicked();

    void on_okButton_clicked();

private:
    Ui::CalcTimestampDlg *ui;

    qint64 nTimestamp;
};

#endif // CALCTIMESTAMPDLG_H
