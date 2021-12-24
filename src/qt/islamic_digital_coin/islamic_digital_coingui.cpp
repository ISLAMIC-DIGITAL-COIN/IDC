// Copyright (c) 2019-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/islamic_digital_coin/islamic_digital_coingui.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include "qt/guiutil.h"
#include "clientmodel.h"
#include "interfaces/handler.h"
#include "optionsmodel.h"
#include "networkstyle.h"
#include "notificator.h"
#include "guiinterface.h"
#include "qt/islamic_digital_coin/qtutils.h"
#include "qt/islamic_digital_coin/defaultdialog.h"

#include "init.h"
#include "util.h"

#include <QApplication>
#include <QColor>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QScreen>
#include <QShortcut>
#include <QWindowStateChangeEvent>


#define BASE_WINDOW_WIDTH 1200
#define BASE_WINDOW_HEIGHT 740
#define BASE_WINDOW_MIN_HEIGHT 620
#define BASE_WINDOW_MIN_WIDTH 1100


const QString ISLAMIC_DIGITAL_COINGUI::DEFAULT_WALLET = "~Default";

ISLAMIC_DIGITAL_COINGUI::ISLAMIC_DIGITAL_COINGUI(const NetworkStyle* networkStyle, QWidget* parent) :
        QMainWindow(parent),
        clientModel(0){

    /* Open CSS when configured */
    this->setStyleSheet(GUIUtil::loadStyleSheet());
    this->setMinimumSize(BASE_WINDOW_MIN_WIDTH, BASE_WINDOW_MIN_HEIGHT);


    // Adapt screen size
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int adaptedHeight = (rec.height() < BASE_WINDOW_HEIGHT) ?  BASE_WINDOW_MIN_HEIGHT : BASE_WINDOW_HEIGHT;
    int adaptedWidth = (rec.width() < BASE_WINDOW_WIDTH) ?  BASE_WINDOW_MIN_WIDTH : BASE_WINDOW_WIDTH;
    GUIUtil::restoreWindowGeometry(
            "nWindow",
            QSize(adaptedWidth, adaptedHeight),
            this
    );

#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !gArgs.GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET

    QString windowTitle = QString::fromStdString(gArgs.GetArg("-windowtitle", ""));
    if (windowTitle.isEmpty()) {
        windowTitle = tr("ISLAMIC DIGITAL COIN") + " - ";
        windowTitle += ((enableWallet) ? tr("Wallet") : tr("Node"));
    }
    windowTitle += " " + networkStyle->getTitleAddText();
    setWindowTitle(windowTitle);

    QApplication::setWindowIcon(networkStyle->getAppIcon());
    setWindowIcon(networkStyle->getAppIcon());

#ifdef ENABLE_WALLET
    // Create wallet frame
    if (enableWallet) {
        QFrame* centralWidget = new QFrame(this);
        this->setMinimumWidth(BASE_WINDOW_MIN_WIDTH);
        this->setMinimumHeight(BASE_WINDOW_MIN_HEIGHT);
        QHBoxLayout* centralWidgetLayouot = new QHBoxLayout();
        centralWidget->setLayout(centralWidgetLayouot);
        centralWidgetLayouot->setContentsMargins(0,0,0,0);
        centralWidgetLayouot->setSpacing(0);

        centralWidget->setProperty("cssClass", "container");
        centralWidget->setStyleSheet("padding:0px; border:none; margin:0px;");

        // First the nav
        navMenu = new NavMenuWidget(this);
        centralWidgetLayouot->addWidget(navMenu);

        this->setCentralWidget(centralWidget);
        this->setContentsMargins(0,0,0,0);

        QFrame *container = new QFrame(centralWidget);
        container->setContentsMargins(0,0,0,0);
        centralWidgetLayouot->addWidget(container);

        // Then topbar + the stackedWidget
        QVBoxLayout *baseScreensContainer = new QVBoxLayout(this);
        baseScreensContainer->setMargin(0);
        baseScreensContainer->setSpacing(0);
        baseScreensContainer->setContentsMargins(0,0,0,0);
        container->setLayout(baseScreensContainer);

        // Insert the topbar
        topBar = new TopBar(this);
        topBar->setContentsMargins(0,0,0,0);
        baseScreensContainer->addWidget(topBar);

        // Now stacked widget
        stackedContainer = new QStackedWidget(this);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        stackedContainer->setSizePolicy(sizePolicy);
        stackedContainer->setContentsMargins(0,0,0,0);
        baseScreensContainer->addWidget(stackedContainer);

        // Init
        dashboard = new DashboardWidget(this);
        sendWidget = new SendWidget(this);
        receiveWidget = new ReceiveWidget(this);
        addressesWidget = new AddressesWidget(this);
        masterNodesWidget = new MasterNodesWidget(this);
        coldStakingWidget = new ColdStakingWidget(this);
        settingsWidget = new SettingsWidget(this);

        // Add to parent
        stackedContainer->addWidget(dashboard);
        stackedContainer->addWidget(sendWidget);
        stackedContainer->addWidget(receiveWidget);
        stackedContainer->addWidget(addressesWidget);
        stackedContainer->addWidget(masterNodesWidget);
        stackedContainer->addWidget(coldStakingWidget);
        stackedContainer->addWidget(settingsWidget);
        stackedContainer->setCurrentWidget(dashboard);

    } else
#endif
    {
        // When compiled without wallet or -disablewallet is provided,
        // the central widget is the rpc console.
        rpcConsole = new RPCConsole(enableWallet ? this : 0);
        setCentralWidget(rpcConsole);
    }

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions(networkStyle);

    // Create system tray icon and notification
    createTrayIcon(networkStyle);

    // Connect events
    connectActions();

    // TODO: Add event filter??
    // // Install event filter to be able to catch status tip events (QEvent::StatusTip)
    //    this->installEventFilter(this);

    // Subscribe to notifications from core
    subscribeToCoreSignals();

}

void ISLAMIC_DIGITAL_COINGUI::createActions(const NetworkStyle* networkStyle)
{
    toggleHideAction = new QAction(networkStyle->getAppIcon(), tr("&Show / Hide"), this);
    toggleHideAction->setStatusTip(tr("Show or hide the main Window"));

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setStatusTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);

    connect(toggleHideAction, &QAction::triggered, this, &ISLAMIC_DIGITAL_COINGUI::toggleHidden);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

/**
 * Here add every event connection
 */
void ISLAMIC_DIGITAL_COINGUI::connectActions()
{
    QShortcut *consoleShort = new QShortcut(this);
    consoleShort->setKey(QKeySequence(SHORT_KEY + Qt::Key_C));
    connect(consoleShort, &QShortcut::activated, [this](){
        navMenu->selectSettings();
        settingsWidget->showDebugConsole();
        goToSettings();
    });
    connect(topBar, &TopBar::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(topBar, &TopBar::themeChanged, this, &ISLAMIC_DIGITAL_COINGUI::changeTheme);
    connect(topBar, &TopBar::onShowHideColdStakingChanged, navMenu, &NavMenuWidget::onShowHideColdStakingChanged);
    connect(settingsWidget, &SettingsWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(sendWidget, &SendWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(receiveWidget, &ReceiveWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(addressesWidget, &AddressesWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(masterNodesWidget, &MasterNodesWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(masterNodesWidget, &MasterNodesWidget::execDialog, this, &ISLAMIC_DIGITAL_COINGUI::execDialog);
    connect(coldStakingWidget, &ColdStakingWidget::showHide, this, &ISLAMIC_DIGITAL_COINGUI::showHide);
    connect(coldStakingWidget, &ColdStakingWidget::execDialog, this, &ISLAMIC_DIGITAL_COINGUI::execDialog);
    connect(settingsWidget, &SettingsWidget::execDialog, this, &ISLAMIC_DIGITAL_COINGUI::execDialog);
}


void ISLAMIC_DIGITAL_COINGUI::createTrayIcon(const NetworkStyle* networkStyle)
{
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("ISLAMIC DIGITAL COIN client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getAppIcon());
    trayIcon->hide();
#endif
    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

ISLAMIC_DIGITAL_COINGUI::~ISLAMIC_DIGITAL_COINGUI()
{
    // Unsubscribe from notifications from core
    unsubscribeFromCoreSignals();

    GUIUtil::saveWindowGeometry("nWindow", this);
    if (trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    MacDockIconHandler::cleanup();
#endif
}


/** Get restart command-line parameters and request restart */
void ISLAMIC_DIGITAL_COINGUI::handleRestart(QStringList args)
{
    if (!ShutdownRequested())
        Q_EMIT requestedRestart(args);
}


void ISLAMIC_DIGITAL_COINGUI::setClientModel(ClientModel* _clientModel)
{
    this->clientModel = _clientModel;
    if (this->clientModel) {
        // Create system tray menu (or setup the dock menu) that late to prevent users from calling actions,
        // while the client has not yet fully loaded
        createTrayIconMenu();

        topBar->setClientModel(clientModel);
        dashboard->setClientModel(clientModel);
        sendWidget->setClientModel(clientModel);
        settingsWidget->setClientModel(clientModel);

        // Receive and report messages from client model
        connect(clientModel, &ClientModel::message, this, &ISLAMIC_DIGITAL_COINGUI::message);
        connect(clientModel, &ClientModel::alertsChanged, [this](const QString& _alertStr) {
            message(tr("Alert!"), _alertStr, CClientUIInterface::MSG_WARNING);
        });
        connect(topBar, &TopBar::walletSynced, dashboard, &DashboardWidget::walletSynced);
        connect(topBar, &TopBar::walletSynced, coldStakingWidget, &ColdStakingWidget::walletSynced);

        // Get restart command-line parameters and handle restart
        connect(settingsWidget, &SettingsWidget::handleRestart, [this](QStringList arg){handleRestart(arg);});

        if (rpcConsole) {
            rpcConsole->setClientModel(clientModel);
        }

        if (trayIcon) {
            trayIcon->show();
        }
    } else {
        // Disable possibility to show main window via action
        toggleHideAction->setEnabled(false);
        if (trayIconMenu) {
            // Disable context menu on tray icon
            trayIconMenu->clear();
        }
    }
}

void ISLAMIC_DIGITAL_COINGUI::createTrayIconMenu()
{
#ifndef Q_OS_MAC
    // return if trayIcon is unset (only on non-macOSes)
    if (!trayIcon)
        return;

    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &ISLAMIC_DIGITAL_COINGUI::trayIconActivated);
#else
    // Note: On macOS, the Dock icon is used to provide the tray's functionality.
    MacDockIconHandler* dockIconHandler = MacDockIconHandler::instance();
    connect(dockIconHandler, &MacDockIconHandler::dockIconClicked, this, &ISLAMIC_DIGITAL_COINGUI::macosDockIconActivated);

    trayIconMenu = new QMenu(this);
    trayIconMenu->setAsDockMenu();
#endif

    // Configuration of the tray icon (or Dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();

#ifndef Q_OS_MAC // This is built-in on macOS
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif
}

#ifndef Q_OS_MAC
void ISLAMIC_DIGITAL_COINGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        // Click on system tray icon triggers show/hide of the main window
        toggleHidden();
    }
}
#else
void ISLAMIC_DIGITAL_COINGUI::macosDockIconActivated()
 {
     show();
     activateWindow();
 }
#endif

void ISLAMIC_DIGITAL_COINGUI::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if (e->type() == QEvent::WindowStateChange) {
        if (clientModel && clientModel->getOptionsModel() && clientModel->getOptionsModel()->getMinimizeToTray()) {
            QWindowStateChangeEvent* wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if (!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized()) {
                QTimer::singleShot(0, this, &ISLAMIC_DIGITAL_COINGUI::hide);
                e->ignore();
            } else if ((wsevt->oldState() & Qt::WindowMinimized) && !isMinimized()) {
                QTimer::singleShot(0, this, &ISLAMIC_DIGITAL_COINGUI::show);
                e->ignore();
            }
        }
    }
#endif
}

void ISLAMIC_DIGITAL_COINGUI::closeEvent(QCloseEvent* event)
{
#ifndef Q_OS_MAC // Ignored on Mac
    if (clientModel && clientModel->getOptionsModel()) {
        if (!clientModel->getOptionsModel()->getMinimizeOnClose()) {
            QApplication::quit();
        } else {
            QMainWindow::showMinimized();
            event->ignore();
        }
    }
#else
    QMainWindow::closeEvent(event);
#endif
}


void ISLAMIC_DIGITAL_COINGUI::messageInfo(const QString& text)
{
    if (!this->snackBar) this->snackBar = new SnackBar(this, this);
    this->snackBar->setText(text);
    this->snackBar->resize(this->width(), snackBar->height());
    openDialog(this->snackBar, this);
}


void ISLAMIC_DIGITAL_COINGUI::message(const QString& title, const QString& message, unsigned int style, bool* ret)
{
    QString strTitle =  tr("ISLAMIC DIGITAL COIN"); // default title
    // Default to information icon
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    // Prefer supplied title over style based title
    if (!title.isEmpty()) {
        msgType = title;
    } else {
        switch (style) {
            case CClientUIInterface::MSG_ERROR:
                msgType = tr("Error");
                break;
            case CClientUIInterface::MSG_WARNING:
                msgType = tr("Warning");
                break;
            case CClientUIInterface::MSG_INFORMATION:
                msgType = tr("Information");
                break;
            default:
                msgType = tr("System Message");
                break;
        }
    }

    // Check for error/warning icon
    if (style & CClientUIInterface::ICON_ERROR) {
        nNotifyIcon = Notificator::Critical;
    } else if (style & CClientUIInterface::ICON_WARNING) {
        nNotifyIcon = Notificator::Warning;
    }

    // Display message
    if (style & CClientUIInterface::MODAL) {
        // Check for buttons, use OK as default, if none was supplied
        int r = 0;
        showNormalIfMinimized();
        if (style & CClientUIInterface::BTN_MASK) {
            r = openStandardDialog(
                    (title.isEmpty() ? strTitle : title), message, "OK", "CANCEL"
                );
        } else {
            r = openStandardDialog((title.isEmpty() ? strTitle : title), message, "OK");
        }
        if (ret != NULL)
            *ret = r;
    } else if (style & CClientUIInterface::MSG_INFORMATION_SNACK) {
        messageInfo(message);
    } else {
        // Append title to "ISLAMIC_DIGITAL_COIN - "
        if (!msgType.isEmpty())
            strTitle += " - " + msgType;
        notificator->notify(static_cast<Notificator::Class>(nNotifyIcon), strTitle, message);
    }
}

bool ISLAMIC_DIGITAL_COINGUI::openStandardDialog(QString title, QString body, QString okBtn, QString cancelBtn)
{
    DefaultDialog *dialog;
    if (isVisible()) {
        showHide(true);
        dialog = new DefaultDialog(this);
        dialog->setText(title, body, okBtn, cancelBtn);
        dialog->adjustSize();
        openDialogWithOpaqueBackground(dialog, this);
    } else {
        dialog = new DefaultDialog();
        dialog->setText(title, body, okBtn);
        dialog->setWindowTitle(tr("ISLAMIC DIGITAL COIN"));
        dialog->adjustSize();
        dialog->raise();
        dialog->exec();
    }
    bool ret = dialog->isOk;
    dialog->deleteLater();
    return ret;
}


void ISLAMIC_DIGITAL_COINGUI::showNormalIfMinimized(bool fToggleHidden)
{
    if (!clientModel)
        return;
    if (!isHidden() && !isMinimized() && !GUIUtil::isObscured(this) && fToggleHidden) {
        hide();
    } else {
        GUIUtil::bringToFront(this);
    }
}

void ISLAMIC_DIGITAL_COINGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void ISLAMIC_DIGITAL_COINGUI::detectShutdown()
{
    if (ShutdownRequested()) {
        if (rpcConsole)
            rpcConsole->hide();
        qApp->quit();
    }
}

void ISLAMIC_DIGITAL_COINGUI::goToDashboard()
{
    if (stackedContainer->currentWidget() != dashboard) {
        stackedContainer->setCurrentWidget(dashboard);
        topBar->showBottom();
    }
}

void ISLAMIC_DIGITAL_COINGUI::goToSend()
{
    showTop(sendWidget);
}

void ISLAMIC_DIGITAL_COINGUI::goToAddresses()
{
    showTop(addressesWidget);
}

void ISLAMIC_DIGITAL_COINGUI::goToMasterNodes()
{
    showTop(masterNodesWidget);
}

void ISLAMIC_DIGITAL_COINGUI::goToColdStaking()
{
    showTop(coldStakingWidget);
}

void ISLAMIC_DIGITAL_COINGUI::goToSettings(){
    showTop(settingsWidget);
}

void ISLAMIC_DIGITAL_COINGUI::goToSettingsInfo()
{
    navMenu->selectSettings();
    settingsWidget->showInformation();
    goToSettings();
}

void ISLAMIC_DIGITAL_COINGUI::goToReceive()
{
    showTop(receiveWidget);
}

void ISLAMIC_DIGITAL_COINGUI::openNetworkMonitor()
{
    settingsWidget->openNetworkMonitor();
}

void ISLAMIC_DIGITAL_COINGUI::showTop(QWidget* view)
{
    if (stackedContainer->currentWidget() != view) {
        stackedContainer->setCurrentWidget(view);
        topBar->showTop();
    }
}

void ISLAMIC_DIGITAL_COINGUI::changeTheme(bool isLightTheme)
{

    QString css = GUIUtil::loadStyleSheet();
    this->setStyleSheet(css);

    // Notify
    Q_EMIT themeChanged(isLightTheme, css);

    // Update style
    updateStyle(this);
}

void ISLAMIC_DIGITAL_COINGUI::resizeEvent(QResizeEvent* event)
{
    // Parent..
    QMainWindow::resizeEvent(event);
    // background
    showHide(opEnabled);
    // Notify
    Q_EMIT windowResizeEvent(event);
}

bool ISLAMIC_DIGITAL_COINGUI::execDialog(QDialog *dialog, int xDiv, int yDiv)
{
    return openDialogWithOpaqueBackgroundY(dialog, this);
}

void ISLAMIC_DIGITAL_COINGUI::showHide(bool show)
{
    if (!op) op = new QLabel(this);
    if (!show) {
        op->setVisible(false);
        opEnabled = false;
    } else {
        QColor bg("#000000");
        bg.setAlpha(200);
        if (!isLightTheme()) {
            bg = QColor("#00000000");
            bg.setAlpha(150);
        }

        QPalette palette;
        palette.setColor(QPalette::Window, bg);
        op->setAutoFillBackground(true);
        op->setPalette(palette);
        op->setWindowFlags(Qt::CustomizeWindowHint);
        op->move(0,0);
        op->show();
        op->activateWindow();
        op->resize(width(), height());
        op->setVisible(true);
        opEnabled = true;
    }
}

int ISLAMIC_DIGITAL_COINGUI::getNavWidth()
{
    return this->navMenu->width();
}

void ISLAMIC_DIGITAL_COINGUI::openFAQ(SettingsFaqWidget::Section section)
{
    showHide(true);
    SettingsFaqWidget* dialog = new SettingsFaqWidget(this);
    dialog->setSection(section);
    openDialogWithOpaqueBackgroundFullScreen(dialog, this);
    dialog->deleteLater();
}


#ifdef ENABLE_WALLET
bool ISLAMIC_DIGITAL_COINGUI::addWallet(const QString& name, WalletModel* walletModel)
{
    // Single wallet supported for now..
    if (!stackedContainer || !clientModel || !walletModel)
        return false;

    // set the model for every view
    navMenu->setWalletModel(walletModel);
    dashboard->setWalletModel(walletModel);
    topBar->setWalletModel(walletModel);
    receiveWidget->setWalletModel(walletModel);
    sendWidget->setWalletModel(walletModel);
    addressesWidget->setWalletModel(walletModel);
    masterNodesWidget->setWalletModel(walletModel);
    coldStakingWidget->setWalletModel(walletModel);
    settingsWidget->setWalletModel(walletModel);

    // Connect actions..
    connect(walletModel, &WalletModel::message, this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(masterNodesWidget, &MasterNodesWidget::message, this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(coldStakingWidget, &ColdStakingWidget::message, this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(topBar, &TopBar::message, this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(sendWidget, &SendWidget::message,this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(receiveWidget, &ReceiveWidget::message,this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(addressesWidget, &AddressesWidget::message,this, &ISLAMIC_DIGITAL_COINGUI::message);
    connect(settingsWidget, &SettingsWidget::message, this, &ISLAMIC_DIGITAL_COINGUI::message);

    // Pass through transaction notifications
    connect(dashboard, &DashboardWidget::incomingTransaction, this, &ISLAMIC_DIGITAL_COINGUI::incomingTransaction);

    return true;
}

bool ISLAMIC_DIGITAL_COINGUI::setCurrentWallet(const QString& name)
{
    // Single wallet supported.
    return true;
}

void ISLAMIC_DIGITAL_COINGUI::removeAllWallets()
{
    // Single wallet supported.
}

void ISLAMIC_DIGITAL_COINGUI::incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address)
{
    // Only send notifications when not disabled
    if (!bdisableSystemnotifications) {
        // On new transaction, make an info balloon
        message((amount) < 0 ? (pwalletMain->fMultiSendNotify == true ? tr("Sent MultiSend transaction") : tr("Sent transaction")) : tr("Incoming transaction"),
            tr("Date: %1\n"
               "Amount: %2\n"
               "Type: %3\n"
               "Address: %4\n")
                .arg(date)
                .arg(BitcoinUnits::formatWithUnit(unit, amount, true))
                .arg(type)
                .arg(address),
            CClientUIInterface::MSG_INFORMATION);

        pwalletMain->fMultiSendNotify = false;
    }
}

#endif // ENABLE_WALLET


static bool ThreadSafeMessageBox(ISLAMIC_DIGITAL_COINGUI* gui, const std::string& message, const std::string& caption, unsigned int style)
{
    bool modal = (style & CClientUIInterface::MODAL);
    // The SECURE flag has no effect in the Qt GUI.
    // bool secure = (style & CClientUIInterface::SECURE);
    style &= ~CClientUIInterface::SECURE;
    bool ret = false;
    std::cout << "thread safe box: " << message << std::endl;
    // In case of modal message, use blocking connection to wait for user to click a button
    QMetaObject::invokeMethod(gui, "message",
              modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
              Q_ARG(QString, QString::fromStdString(caption)),
              Q_ARG(QString, QString::fromStdString(message)),
              Q_ARG(unsigned int, style),
              Q_ARG(bool*, &ret));
    return ret;
}


void ISLAMIC_DIGITAL_COINGUI::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_message_box = interfaces::MakeHandler(uiInterface.ThreadSafeMessageBox.connect(std::bind(ThreadSafeMessageBox, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void ISLAMIC_DIGITAL_COINGUI::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_message_box->disconnect();
}
