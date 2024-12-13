#include "calctimestampdlg.h"
#include "ui_calctimestampdlg.h"
#include <QClipboard>
#include <QMessageBox>

CalcTimestampDlg::CalcTimestampDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalcTimestampDlg)
{
    ui->setupUi(this);

    ui->dateTimelock->setDateTime(QDateTime::currentDateTimeUtc());
    on_buttonConvert_clicked();

}

CalcTimestampDlg::~CalcTimestampDlg()
{
    delete ui;
}

qint64 CalcTimestampDlg::getUnixTimestampFromUTCTime(const QString& timeString)
{
    // Define the format of the input time string
    QString format = "dd/MM/yyyy hh:mm:ss 'UTC'";

    // Parse the time string into a QDateTime object
    QDateTime dateTime = QDateTime::fromString(timeString, format);

    // Set the time zone to UTC
    dateTime.setTimeSpec(Qt::UTC);

    // Convert the QDateTime to a Unix timestamp (seconds since epoch)
    qint64 unixTimestamp = dateTime.toSecsSinceEpoch();

    return unixTimestamp;
}

void CalcTimestampDlg::on_buttonConvert_clicked()
{
    QString timeString = ui->dateTimelock->text();
    qint64 unixTimestamp = getUnixTimestampFromUTCTime(timeString);
    ui->lockStamp->setText(QString::number(unixTimestamp));
}


void CalcTimestampDlg::on_buttonConvert2_clicked()
{
    qint64 unixTimestamp = ui->lockStamp->text().trimmed().toLongLong();

    // Create a QDateTime object from the Unix timestamp
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(unixTimestamp, Qt::UTC);

    // Set the parsed QDateTime into the QDateTimeEdit widget
    ui->dateTimelock->setDateTime(dateTime);
}

void CalcTimestampDlg::on_okButton_clicked()
{
    QString timeString = ui->dateTimelock->text();
    qint64 unixTimestamp = getUnixTimestampFromUTCTime(timeString);
    nTimestamp = (qint64)ui->lockStamp->text().trimmed().toLongLong();
    if (nTimestamp != unixTimestamp)
    {
        QMessageBox::warning(this, tr("Invalid Timestamp"),
                             tr("UTC time and timestamp do not match."),
                             QMessageBox::Ok);
        ui->lockStamp->setValid(false);
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(ui->lockStamp->text());

    accept();
}

QString CalcTimestampDlg::getTimestampString()
{
    return ui->lockStamp->text();
}

void CalcTimestampDlg::setTimestamp(qint64 stamp)
{
    nTimestamp = stamp;
    ui->lockStamp->setText(QString::number(nTimestamp));
    on_buttonConvert2_clicked();
}
