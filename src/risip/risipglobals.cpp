#include "risipglobals.h"
#include "risipphonecontact.h"

#include <QString>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QThreadPool>
#include <QRegularExpression>
#include <QChar>

#include <QDebug>

const char *RisipSettingsParam::QmlUri="Risip";
const QString RisipSettingsParam::FirstRun("firstRun");
const QString RisipSettingsParam::DefaultAccount("defaultAccount");
const QString RisipSettingsParam::AutoSignIn("autoSignIn");
const QString RisipSettingsParam::AccountGroup("SIP_Accounts");
const QString RisipSettingsParam::TotalAccounts("totalAccounts");
const QString RisipSettingsParam::Uri("uri");
const QString RisipSettingsParam::Username("username");
const QString RisipSettingsParam::Password("password");
const QString RisipSettingsParam::ServerAddress("serveraddress");
const QString RisipSettingsParam::NetworkType("networktype");
const QString RisipSettingsParam::Scheme("scheme");
const QString RisipSettingsParam::ProxyServer("proxyServer");
const QString RisipSettingsParam::LocalPort("localPort");
const QString RisipSettingsParam::RandomLocalPort("randomLocalPort");

Country::Country(const QString &id, const QString &name,
                 const QString &code, const QString &prefix)
    :countryId(id)
    ,name(name)
    ,code(code)
    ,prefix(prefix)
{

}

Country::Country(const Country &country)
    :countryId(country.countryId)
    ,name(country.name)
    ,code(country.code)
    ,prefix(country.prefix)
{

}

Country &Country::operator=(const Country &country)
{
    countryId = country.countryId;
    name = country.name;
    code = country.code;
    prefix = country.prefix;

    return *this;
}

bool Country::operator!=(const Country &country)
{
    return (code != country.code);
}

bool Country::operator==(const Country &country)
{
    return (code == country.code);
}

class RisipConfigisLoader: public QRunnable
{
public:
    RisipConfigisLoader()
        :QRunnable()
    {}

    ~RisipConfigisLoader()
    {}

private:
    void run()
    {
        //loading country list
        QFile file(QString(":/configs/countries"));
        if(file.open(QFile::ReadOnly)) {
            QString countryLine;
            QTextStream streamReader(&file);
            QHash<QString, Country> countries;

            Country country;
            while (streamReader.readLineInto(&countryLine)) {

                country.countryId = countryLine.left(countryLine.indexOf(";")).trimmed();

                countryLine.remove(0, countryLine.indexOf(";") +1);
                country.name = countryLine.left(countryLine.indexOf(";")).trimmed();

                countryLine.remove(0, countryLine.indexOf(";") +1);
                country.prefix = countryLine.left(countryLine.indexOf(";")).trimmed();

                countryLine.remove(0, countryLine.indexOf(";") +1);
                country.code = countryLine.left(countryLine.indexOf(";")).trimmed();

                if(!country.flag.load(QString(":/images/icons/flags/550/") + country.code.toLower() + QString(".png")))
                    qDebug()<<"ERROR FLAG LOADED" << QString(":/images/icons/flags/550/") + country.code.toLower() + QString(".png");

                countries.insert(country.prefix, country);
            }

            RisipGlobals::setCountryList(countries);
        } else {
            qDebug()<<"Configs files cannot be found nor be read!!";
        }
    }
};

QHash<QString, Country> RisipGlobals::m_allCountries;
bool RisipGlobals::m_countriesIntialized = false;
RisipGlobals::RisipGlobals()
{

}

QString RisipGlobals::formatToSip(const QString &contact, const QString &server)
{
    QString uri = QString("<sip:")
            +contact
            +QString("@")
            +server
            +QString(">");

    return uri;
}

QList<Country> RisipGlobals::countries()
{
    return m_allCountries.values();
}

const Country &RisipGlobals::countryForPrefix(const QString &prefix)
{
    if(m_allCountries.contains(prefix))
        return m_allCountries[prefix];

    return Country();
}

/**
 * @brief RisipGlobals::initializeCountries
 *
 * MUST call this function at least ONCE in your application, during the startup/initializtion.
 * This function will create the country list to be available application wide.
 */
void RisipGlobals::initializeCountries()
{
    if(!m_countriesIntialized) {
        RisipConfigisLoader *configsLoader = new RisipConfigisLoader;
        configsLoader->setAutoDelete(true);
        QThreadPool::globalInstance()->start(configsLoader);
    }
}

/**
 * @brief RisipGlobals::countriesInitialized
 * @return
 *
 * Returns true/false whether the country code list is ready.
 */
bool RisipGlobals::countriesInitialized()
{
    return m_countriesIntialized;
}

void RisipGlobals::validateNumber(RisipPhoneNumber *number)
{
    if(!number)
        return;

    QString rawNumber = number->rawNumber();
    if(rawNumber.isEmpty())
        return;

    rawNumber = rawNumber.normalized(QString::NormalizationForm_KC, QChar::Unicode_8_0);
    rawNumber.trimmed();
    rawNumber.remove(QString("+"));
    rawNumber.remove(QString(QString("-")));
    rawNumber.simplified();
    QStringList parts = rawNumber.split(QRegularExpression("\\s+"));

    Country country;
    if(parts.count() == 1) {

        if(m_allCountries.contains(parts.first().left(1)))
            country = m_allCountries[parts.first().left(1)];
        else if(m_allCountries.contains(parts.first().left(2)))
            country = m_allCountries[parts.first().left(2)];
        else if(m_allCountries.contains(parts.first().left(3)))
            country = m_allCountries[parts.first().left(3)];
        else if(m_allCountries.contains(parts.first().left(4)))
            country = m_allCountries[parts.first().left(4)];

    } else if(parts.count() == 2) {
        if(m_allCountries.contains(parts.first()))
            country = m_allCountries[parts.first()];
    } else if(parts.count() >= 3) {
        if(m_allCountries.contains(parts.first()))
            country = m_allCountries[parts.first()];
    }

    number->setCountryPrefix(country.prefix);
    number->setCountryCode(country.code);
    number->setCountryName(country.name);
    number->setFullNumber(rawNumber.remove(QRegularExpression("\\s+")));
    number->setNumberParts(parts);
}

void RisipGlobals::setCountryList(QHash<QString, Country> countryList)
{
    m_allCountries = countryList;
    if(!m_allCountries.isEmpty())
        m_countriesIntialized = true;
    else
        m_countriesIntialized = false;
}