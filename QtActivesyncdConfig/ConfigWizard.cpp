// Class include
#include "ConfigWizard.h"
#ifdef HAVE_CONFIG_H /* In an autotools build */
#include "ConfigWizard.ui.h"
#else
#include "ui_ConfigWizard.h"
#endif
// System includes
#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>
// User includes

// TODO: MUSTN'T link to libeas directly: replace with a client-side API
#include "../eas-daemon/libeas/eas-connection.h"
#include "../libeasaccount/src/eas-account.h"
#include "../libeasaccount/src/eas-account-list.h"
#include "../libeasmail/src/libeasmail.h"


extern ConfigWizard* theWizard;


/**
 * Global callback, called after attempt at auto-discovery
 */
void autoDiscoverCallback(char* server_uri, void* /*data*/, GError* /*error*/)
{
    qDebug("Entering autoDiscoverCallback() with server_uri=%s", (server_uri ? server_uri : "0"));

    if (server_uri)
    {
        QString qServerUri(server_uri);
        if (!qServerUri.isEmpty())
        {
            theWizard->storeServerDetails(qServerUri);
            return;
        }
    }

    theWizard->onAutoDiscoverFailure();
}


/**
 * Constructor
 */
ConfigWizard::ConfigWizard(QWidget *parent)
: QDialog(parent),
  ui(new Ui::ConfigWizard),
  serverDetailsEnteredManually(false)
{
    ui->setupUi(this);

    ui->editEmailAddress->setValidator(new QRegExpValidator(QRegExp("^\\S{2,}@\\S{2,}\\.\\S{2,}"), ui->editEmailAddress));
    ui->editServerUri->setValidator(new QRegExpValidator(QRegExp("^\\S{2,}\\.\\S{2,}"), ui->editServerUri));
    ui->editUsername2->setValidator(new QRegExpValidator(QRegExp("\\S+"), ui->editUsername2));

    // Connect signals/slots
    connect(ui->btnNext, SIGNAL(clicked()), SLOT(onNext()));
    connect(ui->btnBack, SIGNAL(clicked()), SLOT(onBack()));
    connect(ui->btnCancel, SIGNAL(clicked()), SLOT(onCancel()));
    connect(ui->editEmailAddress, SIGNAL(textEdited(QString)), SLOT(validateAutoDiscoverInputs()));
    connect(ui->editServerUri, SIGNAL(textEdited(QString)), SLOT(validateManualServerInputs()));
    connect(ui->editUsername2, SIGNAL(textEdited(QString)), SLOT(validateManualServerInputs()));
    // Connect the two username fields
    connect(ui->editUsername1, SIGNAL(textChanged(QString)), ui->editUsername2, SLOT(setText(QString)));
    connect(ui->editUsername2, SIGNAL(textChanged(QString)), ui->editUsername1, SLOT(setText(QString)));

    changeState(AutoDiscoverDetails);
}


/**
 * Destructor
 */
ConfigWizard::~ConfigWizard()
{
    delete ui;
}


/**
 * Slot: next button clicked
 */
void ConfigWizard::onNext()
{
    switch (currentState)
    {
    case AutoDiscoverDetails:
        changeState(TryingAutoDiscover);
        break;

    case ManualServerDetails:
        username = ui->editUsername2->text().trimmed();
        storeServerDetails(ui->editServerUri->text().trimmed());
        break;

    case ConfirmProvisionReqts:
        changeState(Finish);
        break;

    case Finish:
        // TODO: apply settings
        QApplication::quit();
        break;

    default:
        break;
    }
}


/**
 * Slot: back button clicked
 */
void ConfigWizard::onBack()
{
    switch (currentState)
    {
    case TryingAutoDiscover:
    case ManualServerDetails:
        changeState(AutoDiscoverDetails);
        break;

    case GettingProvisionReqts:
    case ConfirmProvisionReqts:
        changeState(serverDetailsEnteredManually ? ManualServerDetails : AutoDiscoverDetails);
        break;

    case Finish:
        changeState(ConfirmProvisionReqts);
        break;

    default:
        break;
    }
}


/**
 * Slot: cancel button clicked
 */
void ConfigWizard::onCancel()
{
    QApplication::quit();
}


/**
 * Slot: Auto-discovery or manual server entry completed successfully
 */
void ConfigWizard::storeServerDetails(const QString& uri)
{
    qDebug() << "Entering storeServerDetails() with uri=" << uri;
    serverUri = uri;

    // TODO: ADD BETTER ERROR HANDLING?

    // Store the account details in GConf
    GConfClient* gconfClient = gconf_client_get_default();
    if (gconfClient)
    {
        EasAccountList* easAccountList = eas_account_list_new(gconfClient);
        if (easAccountList)
        {
            EasAccount* easAccount = eas_account_new();
            if (easAccount)
            {
                eas_account_set_uid(easAccount, (const gchar*)emailAddress.toUtf8().constData());
                eas_account_set_uri(easAccount, (const gchar*)serverUri.toUtf8().constData());
                eas_account_set_username(easAccount, (const gchar*)username.toUtf8().constData());
                eas_account_list_save_account(easAccountList, easAccount);
                g_object_unref(easAccount);
            }
            else // easAccount is null
            {
                qWarning() << "Failed to create new EasAccountList in ConfigWizard::storeServerDetails()";
            }

            g_object_unref(easAccountList);
        }
        else // easAccountList is null
        {
            qWarning() << "Failed to create new EasAccount in ConfigWizard::storeServerDetails()";
        }

        g_object_unref(gconfClient);
    }
    else // gconfClienf is null
    {
        qWarning() << "Failed to create new GConfClient in ConfigWizard::storeServerDetails()";
    }

    getProvisionReqts();
}


/**
 * Slot: auto-discovery failed
 */
void ConfigWizard::onAutoDiscoverFailure()
{
    changeState(ManualServerDetails);
}


/**
 * Slot
 */
void ConfigWizard::onProvisionFailure()
{

}


/**
 * Slot
 */
void ConfigWizard::onProvisionSuccess()
{

}


/**
 * Slot: text in one of the auto-config inputs has changed
 */
void ConfigWizard::validateAutoDiscoverInputs()
{
    ui->btnNext->setEnabled(ui->editEmailAddress->hasAcceptableInput());
}


/**
 * Slot: text in one of the manual config inputs has changed
 */
void ConfigWizard::validateManualServerInputs()
{
    ui->btnNext->setEnabled(ui->editServerUri->hasAcceptableInput() && ui->editUsername2->hasAcceptableInput());
}


/**
 * Switch to a specific page in the wizard
 */
void ConfigWizard::changeState(ConfigWizard::State state)
{
    qDebug() << "Entering changeState";

    currentState = state;

    // Reset the button captions to defaults
    setButtonCaptions();

    // Set button defaults
    ui->btnBack->setEnabled(state != AutoDiscoverDetails); // Enabled on all but the first page
    ui->btnNext->setEnabled(true);
    ui->btnCancel->setEnabled(true);

    switch (currentState)
    {
    case AutoDiscoverDetails:
        ui->wizard->setCurrentWidget(ui->pageAutoConfig);
        setTitle(tr("Automatic configuration"), tr("Enter your e-mail adress and we'll try to guess your Exchange server details."));
        validateAutoDiscoverInputs();
        break;

    case TryingAutoDiscover:
        qDebug() << "Entering state: TryingAutoDiscover";
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Attempting automatic configuration"), tr("Please wait..."));

        qDebug() << "GOT TO HERE";

        // Use username if entered, otherwise use the e-mail address
        emailAddress = ui->editEmailAddress->text();
        username = ui->editUsername1->text();
        qDebug() << "Calling autodiscover with email address" << emailAddress << "username" << username;

        eas_connection_autodiscover(
            autoDiscoverCallback, 0,
            (const gchar*)emailAddress.constData(),
            (username.isEmpty() ? 0 : (const gchar*)username.constData()));
        break;

    case ManualServerDetails:
        serverDetailsEnteredManually = true;
        ui->wizard->setCurrentWidget(ui->pageManualConfig);
        setTitle(tr("Manual configuration"), tr("Sorry, your Exchange server details could not be guessed. Please enter them below."));
        validateManualServerInputs();
        break;

    case GettingProvisionReqts:
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Getting server provisioning requirements"), tr("Please wait..."));
        break;

    case ConfirmProvisionReqts:
        {
        ui->wizard->setCurrentWidget(ui->pageConfirmRequirements);
        setTitle(tr("Confirm ActiveSync requirements"), tr("The server at <b>%1</b> requires you to accept the following features before continuing.").arg(ui->editServerUri->text().trimmed()));
        setButtonCaptions(tr("Accept"));

        // TEMP
        QStringList items;
        items << tr("Remote wipe")
              << tr("Disable all other POP and IMAP e-mail accounts on this device (and by the way, isn't this a long item?)")
              << tr("Offer up first-born")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another");
        ui->listRequirements->addItems(items);
        }
        break;

    case Finish:
        ui->wizard->setCurrentWidget(ui->pageFinish);
        setTitle(tr("ActveSync is now configured"), tr("Please confirm which other services you would like to synchronise with this device."));
        setButtonCaptions(tr("Finish"));
        ui->btnCancel->setEnabled(false);
        break;

    case Error:
        ui->wizard->setCurrentWidget(ui->pageError);
        setTitle(tr("Error"), tr("Sorry, something has gone wrong."));
        setButtonCaptions(tr("Quit"));
        break;

    default:
        break;
    }
}


/**
 * Contact the server and get the provisioning requirements (if any)
 */
void ConfigWizard::getProvisionReqts()
{
    changeState(GettingProvisionReqts);

    GError* error = 0;
    EasEmailHandler* mailHandler = eas_mail_handler_new(emailAddress.toUtf8().constData(), &error);
    if (error)
    {
        g_error_free(error);
        error = 0;
    }
    if (mailHandler)
    {
        GSList* folderList = 0;
        gboolean ret = eas_mail_handler_get_folder_list(mailHandler, true, &folderList, 0, &error);

        // TODO: decide how to deal with this. Do we even need to call ...get_folder_list()?
        // Will ...get_provision_list() on its own do?

        if (error)
        {
            g_error_free(error);
            error = 0;
        }


        // TODO:
        // eas_mail_handler_get_provision_list()...
        // eas_mail_handler_accept_provision_list()...

        g_object_unref(mailHandler);
    }
    else // mailHandler is null
    {
        qWarning() << "Failed to construct create new EasEmailHandler in ConfigWizard::changeState()";
    }
}


/**
 * Set the wizard title & subtitle fields
 */
void ConfigWizard::setTitle(const QString title, const QString& subTitle)
{
    ui->lblTitle->setText(title);
    ui->lblSubTitle->setText(subTitle);
}


/**
 * Set the wizard button captions
 */
void ConfigWizard::setButtonCaptions(const QString& nextButtonCaption, const QString& backButtonCaption)
{
    ui->btnNext->setText(nextButtonCaption.isEmpty() ? tr("Next") : nextButtonCaption);
    ui->btnBack->setText(backButtonCaption.isEmpty() ? tr("Back") : backButtonCaption);
}


void ConfigWizard::error(const QString& msg)
{
    ui->lblErrorDetails->setText(msg);
    changeState(Error);
}
