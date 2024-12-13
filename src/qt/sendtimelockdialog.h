#ifndef SENDTIMELOCKDIALOG_H
#define SENDTIMELOCKDIALOG_H

#include <QDialog>
#include "walletmodel.h"
#include "clientmodel.h"

namespace Ui {
class sendtimelockdialog;
}

class sendtimelockdialog : public QDialog
{
    Q_OBJECT

public:
    explicit sendtimelockdialog(QWidget *parent = nullptr);
    ~sendtimelockdialog();

    void setModels(WalletModel *mw, ClientModel *mc);
    void setBlockHeight(int height);

private:
    bool validate();

    uint256 ConvertHexstringToUint256(const QString &transactionHex);

private slots:
    void on_addrButton_clicked();
    void on_buttonGetHeight_clicked();
    void on_buttonCalcTime_clicked();
    void on_radioHeight_toggled(bool checked);
    void on_radioStamp_toggled(bool checked);
    void on_sendButton_clicked();
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 reserveBalance);
    void setNumBlocks(int count, int countOfPeers);


private:
    Ui::sendtimelockdialog *ui;
    WalletModel *wModel;
    ClientModel *cModel;
};

#endif // SENDTIMELOCKDIALOG_H
