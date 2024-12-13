#include "sendtimelockdialog.h"
#include "ui_sendtimelockdialog.h"
#include "optionsmodel.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "coincontroldialog.h"
#include "sendcoinsentry.h"
#include "calctimestampdlg.h"
#include "main.h"
#include "addresstablemodel.h"

#include <QDateTime>
#include <QDoubleSpinBox>
#include <QMessageBox>


const unsigned int MAX_UNIX_TIMESTAMP = std::numeric_limits<unsigned int>::max();

sendtimelockdialog::sendtimelockdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sendtimelockdialog),
    wModel(0),
    cModel(0)
{
    ui->setupUi(this);

    ui->radioHeight->setChecked(true);

#ifdef Q_OS_WIN
    resize(350, 300);
    setMinimumSize(350, 300);
#elif defined Q_OS_MAC
    resize(460, 300);
    setMinimumSize(460, 300);
#else
    resize(400, 330);
    setMinimumSize(400, 330);
#endif
}

sendtimelockdialog::~sendtimelockdialog()
{
    delete ui;
}

void sendtimelockdialog::setModels(WalletModel *mw, ClientModel *mc)
{
    wModel = mw;

    if(wModel)
    {
        setBalance(wModel->getBalance(), 0, 0, 0);
        connect(wModel, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));
    }

    cModel = mc;
    if (cModel)
    {
        setNumBlocks(cModel->getNumBlocks(), 0);
        connect(cModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));
    }
}

void sendtimelockdialog::on_radioHeight_toggled(bool checked)
{
    if (checked)
    {
        ui->lockHeight->show();
        ui->buttonGetHeight->show();
        ui->lockStamp->hide();
        ui->buttonCalcTime->hide();
    }
}


void sendtimelockdialog::on_radioStamp_toggled(bool checked)
{
    if (checked)
    {
        ui->lockHeight->hide();
        ui->buttonGetHeight->hide();
        ui->lockStamp->show();
        ui->buttonCalcTime->show();
    }
}

bool sendtimelockdialog::validate()
{
    // Check input validity
    bool retval = true;

    if (!ui->payTo->hasAcceptableInput() || (wModel && !wModel->validateAddress(ui->payTo->text())))
    {
        ui->payTo->setValid(false);
        retval = false;
    }


    if (!ui->payAmount->validate())
        retval = false;
    else
    {
        qint64 sendBalance = ui->payAmount->value();
        if (sendBalance <= 0)
        {
            // Cannot send 0 coins or less
            ui->payAmount->setValid(false);
            retval = false;
        }
        else
        {
            qint64 maxBalance = wModel->getBalance();
            if (sendBalance > maxBalance)
            {
                ui->payAmount->setValid(false);
                retval = false;
            }
        }
    }

    return retval;
}

void sendtimelockdialog::setBlockHeight(int height)
{
    ui->labelHeight->setText(tr("Latest Blocks: ") + QString::number(height));
}

void sendtimelockdialog::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 reserveBalance)
{
    Q_UNUSED(stake);
    Q_UNUSED(unconfirmedBalance);
    Q_UNUSED(reserveBalance);
    if (!wModel || !wModel->getOptionsModel())
        return;

    int unit = wModel->getOptionsModel()->getDisplayUnit();
    ui->labelBalance->setText(tr("Balance: ") +BitcoinUnits::formatWithUnit(unit, balance));
}

void sendtimelockdialog::setNumBlocks(int count, int countOfPeers)
{
    Q_UNUSED(countOfPeers);
    setBlockHeight(count);
}

void sendtimelockdialog::on_addrButton_clicked()
{
    if(!wModel)
        return;
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
    dlg.setModel(wModel->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}

void sendtimelockdialog::on_sendButton_clicked()
{
    if (!validate())
        return;

    uint32_t locktime = 0;

    if (ui->radioHeight->isChecked())
    {
        unsigned int latestBlockHeight = cModel->getNumBlocks();
        unsigned int uiHeight = ui->lockHeight->text().trimmed().toInt();

        if (uiHeight <= latestBlockHeight || uiHeight > LOCKTIME_THRESHOLD)
        {
            ui->lockHeight->setValid(false);
            QMessageBox::warning(this, tr("Invalid Block Height"),
                                 tr("The block height must be greater than the current block height and less than %1.").arg(LOCKTIME_THRESHOLD),
                                 QMessageBox::Ok);
            return;
        }

        locktime = (unsigned int)uiHeight;
    }
    else if (ui->radioStamp->isChecked())
    {
        qint64 currentTimestamp = QDateTime::currentSecsSinceEpoch();
        qint64 unixTimestamp = ui->lockStamp->text().trimmed().toLongLong();

        if (unixTimestamp <= currentTimestamp || unixTimestamp > MAX_UNIX_TIMESTAMP)
        {
           ui->lockStamp->setValid(false);
           QMessageBox::warning(this, tr("Invalid Timestamp"),
                                tr("The timestamp must be in the future and less than %1.").arg(MAX_UNIX_TIMESTAMP),
                                QMessageBox::Ok);
           return;
        }

        locktime = (unsigned int)unixTimestamp;
    }

    QString formattedMsg = tr("Are you sure you want to send <b>%1</b> to %2?")
                        .arg(BitcoinUnits::formatWithUnit(wModel->getOptionsModel()->getDisplayUnit(), ui->payAmount->value()), ui->payTo->text());

    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Confirm send coins with timelock"),
                                                            formattedMsg,
                                                            QMessageBox::Yes|QMessageBox::Cancel,
                                                            QMessageBox::Cancel);

    if (ret != QMessageBox::Yes)
        return;


    WalletModel::UnlockContext ctx(wModel->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        return;
    }

    WalletModel::SendCoinsReturn sendstatus;

    QList<SendCoinsRecipient> recipients;

    SendCoinsRecipient rv;
    rv.address = ui->payTo->text();
    rv.label = wModel->getAddressTableModel()->labelForAddress(rv.address);
    rv.amount = ui->payAmount->value();
    recipients.append(rv);

    sendstatus = wModel->sendCoins(recipients, NULL, locktime);

    QString title = tr("Send coins with timelock");

    switch (sendstatus.status)
    {
    case WalletModel::InvalidAddress:
        QMessageBox::warning(this, title, tr("The recipient address is not valid, please recheck."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::InvalidAmount:
        QMessageBox::warning(this, title, tr("The amount to pay must be larger than 0."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountExceedsBalance:
        QMessageBox::warning(this, title, tr("The amount exceeds your balance."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        QMessageBox::warning(this, title, tr("The total exceeds your balance when the %1 transaction fee is included.").
            arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, sendstatus.fee)),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::DuplicateAddress:
        QMessageBox::warning(this, title, tr("Duplicate address found, can only send to each address once per send operation."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCreationFailed:
        QMessageBox::warning(this, title, tr("Error: Transaction creation failed."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCommitFailed:
        QMessageBox::warning(this, title,
            tr("Error: The transaction was rejected. This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::BadBurningCoins: // there is no such thing as BadBurningCoins when burning coins
      break;
    case WalletModel::Aborted: // User aborted, nothing to do
        break;
    case WalletModel::OK:
        accept();
        break;
    }
}

uint256 sendtimelockdialog::ConvertHexstringToUint256(const QString& str)
{
    uint256 txid;
    txid.SetHex(str.toStdString());
    return txid;
}

void sendtimelockdialog::on_buttonGetHeight_clicked()
{
    int height = cModel->getNumBlocks();
    ui->lockHeight->setText(QString::number(height));
}


void sendtimelockdialog::on_buttonCalcTime_clicked()
{
    CalcTimestampDlg dlg(this);

    qint64 unixTimestamp = ui->lockStamp->text().trimmed().toLongLong();
    if (unixTimestamp > LOCKTIME_THRESHOLD)
        dlg.setTimestamp(unixTimestamp);

    if (dlg.exec() == QDialog::Accepted) {
        ui->lockStamp->setText(dlg.getTimestampString());
    } else {
        ui->lockStamp->setText("");
    }
}


