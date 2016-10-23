////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is part of QOAuth2.                                              //
// Copyright (c) 2014 Jacob Dawid <jacob@omg-it.works>                        //
//                                                                            //
// QOAuth2 is free software: you can redistribute it and/or modify            //
// it under the terms of the GNU Affero General Public License as             //
// published by the Free Software Foundation, either version 3 of the         //
// License, or (at your option) any later version.                            //
//                                                                            //
// QOAuth2 is distributed in the hope that it will be useful,                 //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              //
// GNU Affero General Public License for more details.                        //
//                                                                            //
// You should have received a copy of the GNU Affero General Public           //
// License along with QOAuth2.                                                //
// If not, see <http://www.gnu.org/licenses/>.                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef OAUTH2SERVICE_H
#define OAUTH2SERVICE_H

// Qt includes
#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>

/**
 * Service for interfacing the OAuth2 device API.
 * https://developers.google.com/accounts/docs/OAuth2ForDevices
 *
 * The authorization process takes two steps. First, call retrieveUserCode()
 * and wait for the userAuthorizationRequired signal. You are supposed to
 * navigate the verification url and let the user enter the user code, for
 * example by opening the url in a webbrowser.
 *
 */
class OAuth2Service : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new OAuth2 service object.
     * @param userCodeUrl e.g. "https://accounts.google.com/o/oauth2/device/code".
     * @param tokenUrl e.g. "https://accounts.google.com/o/oauth2/token".
     * @param clientId Your client id, @see Google Developer Console.
     * @param clientSecret Your client secret, @see Google Developer Console.
     * @param scope The access scope requested.
     * @param parent The parent object.
     */
    explicit OAuth2Service(QString userCodeUrl,
                           QString tokenUrl,
                           QString clientId,
                           QString clientSecret,
                           QString scope,
                           QObject *parent = 0);

    /**
     * Sets the user agent to be used when communicating with OAuth servers.
     * @param userAgent
     */
    void setUserAgent(QString userAgent);

    /** @returns the current user agent used to communicate with OAuth servers. */
    QString userAgent();

    /**
     * Sets the OAuth grant type.
     * @param oAuthTokenGrantType
     */
    void setOAuthTokenGrantType(QString oAuthTokenGrantType);

    /** @returns the OAuth grant type. */
    QString oAuthTokenGrantType();

signals:
    /**
     * The server prompts you to authenticate your application.
     * @param deviceCode Store the device code to query API tokens.
     * @param userCode The user is supposed to enter the userCode at the given verificationUrl.
     * @param verificationUrl The url that the user code needs to be entered at.
     * @param expiresIn
     * @param interval
     */
    void userAuthorizationRequired(QString deviceCode,
                                   QString userCode,
                                   QString verificationUrl,
                                   int expiresIn,
                                   int interval);

    /**
     * The server sent you the queried access token.
     * @param accessToken The API access token. You will need this to access the queried API.
     * @param tokenType The token type.
     * @param expiresIn
     * @param refreshToken The refresh token. You will need this to renew the accessToken once it has expired. May be empty.
     */
    void accessTokenReceived(QString accessToken,
                             QString tokenType,
                             int expiresIn,
                             QString refreshToken);

    void tokenRetrieveError(QString error, QString errorDescription);


public slots:
    /**
     * First time app setup. Initiates a query to start authorization. You will
     * need the store the provided deviceCode to request an access token.
     * @see userAuthorizationRequired
     */
    void retrieveUserCode();

    /**
     * First retrieval of access token. Use the device code to retrieve an API
     * access token for the first time. You will be provided with refresh token,
     * which you need to store in case the access token expires. Once you have
     * redeemed the access token via this method, it is not possible to obtain
     * the access token again.
     */
    void retrieveAccessToken(QString deviceCode);

    /**
     * In case you access token expires, use this method to refresh your access
     * token.
     * @param refreshToken the refresh token obtained via retrieveAccessToken.
     */
    void refreshAccessToken(QString refreshToken);

private slots:
    void userCodeRequestFinished(QNetworkReply* networkReply);
    void tokenRequestFinished(QNetworkReply* networkReply);

private:
    /** The user agent used when communicating with OAuth servers. */
    QString _userAgent;
    /** The OAuth token grant type. */
    QString _oAuthTokenGrantType;

    /** The client Id. */
    QString _clientId;
    /** The client secret. */
    QString _clientSecret;

    /** The user code API endpoint. */
    QUrl _userCodeAPIEndpoint;
    /** The token API endpoint. */
    QUrl _tokenAPIEndpoint;

    /** The authorization scope. */
    QString _scope;

    /** Access manager to perform user code API endpoint queries. */
    QNetworkAccessManager _userCodeNetworkAccessManager;
    /** Access manager to perform token API endpoint queries. */
    QNetworkAccessManager _tokenNetworkAccessManager;
};

/**
 * Preconfigured URLs for Google OAuth2.
 */
class GoogleOAuth2Service : public OAuth2Service {
    Q_OBJECT
public:
    /** Constructor */
    GoogleOAuth2Service(QString clientId,
                        QString clientSecret,
                        QString scope,
                        QObject *parent = 0);

    ~GoogleOAuth2Service();
};

#endif // OAUTH2SERVICE_H
