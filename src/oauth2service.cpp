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

// Own includes
#include "oauth2service.h"

// Qt includes
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

OAuth2Service::OAuth2Service(QString userCodeUrl,
                             QString tokenUrl,
                             QString clientId,
                             QString clientSecret,
                             QString scope,
                             QObject *parent) :
    QObject(parent)
{
    _userAgent = "qt-oauth2/1.0";
    _oAuthTokenGrantType = "authorization_code";

    _userCodeAPIEndpoint = QUrl(userCodeUrl);
    _tokenAPIEndpoint = QUrl(tokenUrl);

    _clientId = clientId;
    _clientSecret = clientSecret;

    _scope = scope;

    connect(&_userCodeNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(userCodeRequestFinished(QNetworkReply*)));
    connect(&_tokenNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(tokenRequestFinished(QNetworkReply*)));
}

void OAuth2Service::setUserAgent(QString userAgent)
{
    _userAgent = userAgent;
}

QString OAuth2Service::userAgent()
{
    return _userAgent;
}

void OAuth2Service::setOAuthTokenGrantType(QString oAuthTokenGrantType)
{
    _oAuthTokenGrantType = oAuthTokenGrantType;
}

QString OAuth2Service::oAuthTokenGrantType()
{
    return _oAuthTokenGrantType;
}

void OAuth2Service::retrieveUserCode()
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(_userCodeAPIEndpoint);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    //networkRequest.setHeader(QNetworkRequest::UserAgentHeader, _userAgent);

    QString content = QString("client_id=%1&scope=%2&response_type=%3&redirect_uri=%4")
            .arg(_clientId)
            .arg(_scope)
            .arg("code")
            .arg("http://127.0.0.1");


// https://accounts.google.com/o/oauth2/auth?client_id=554660036943-ous5oec45o9k567rs1adr8sc9thuvnek.apps.googleusercontent.com&scope=https://www.googleapis.com/auth/drive.readonly&response_type=code&redirect_uri=http://127.0.0.1
/* https://accounts.google.com/o/oauth2/auth?
 * client_id=554660036943-ous5oec45o9k567rs1adr8sc9thuvnek.apps.googleusercontent.com&
 * scope=https://www.googleapis.com/auth/drive.readonly&
 * response_type=code&
 * redirect_uri=http://127.0.0.1
*/

    _userCodeNetworkAccessManager.post(networkRequest, content.toLocal8Bit());
}

void OAuth2Service::retrieveAccessToken(QString deviceCode)
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(_tokenAPIEndpoint);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
//    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, _userAgent);

    QString content = QString("client_id=%1&"
                              "client_secret=%2&"
                              "code=%3&"
                              "grant_type=%4&"
                              "redirect_uri=%5")
            .arg(_clientId)
            .arg(_clientSecret)
            .arg(deviceCode)
            .arg(_oAuthTokenGrantType)
            .arg("https://127.0.0.1/code");

// https://accounts.google.com/o/oauth2/token?client_id=554660036943-ous5oec45o9k567rs1adr8sc9thuvnek.apps.googleusercontent.com&client_secret=V8nPoYK7LC-x21IRNj-4zZjO&code=4/tnJFB4D_nhAQQUkqc3bZwelgAzz_MslVGuXLU6d_dzc&grant_type=authorization_code&redirect_uri=https://127.0.0.1/code
/* https://accounts.google.com/o/oauth2/token?
 * client_id=554660036943-ous5oec45o9k567rs1adr8sc9thuvnek.apps.googleusercontent.com&
 * client_secret=V8nPoYK7LC-x21IRNj-4zZjO&
 * code=4/G614Mer3MFoOvFs3SLZUojkMwdRyFdoebEVIzmw9WZY&
 * grant_type=authorization_code&
 * redirect_uri=https://127.0.0.1/code
*/

    _tokenNetworkAccessManager.post(networkRequest, content.toUtf8());
}

void OAuth2Service::refreshAccessToken(QString refreshToken)
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(_tokenAPIEndpoint);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, _userAgent);

    QString content = QString("client_id=%1&"
                              "client_secret=%2&"
                              "refresh_token=%3&"
                              "grant_type=%4")
            .arg(_clientId)
            .arg(_clientSecret)
            .arg(refreshToken)
            .arg("refresh_token");

    _tokenNetworkAccessManager.post(networkRequest, content.toUtf8());
}

void OAuth2Service::userCodeRequestFinished(QNetworkReply* networkReply)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(networkReply->readAll());
    QJsonObject rootObject = jsonDocument.object();
    qDebug() << "User code response:";
    qDebug() << jsonDocument.toJson();
    emit userAuthorizationRequired(
                rootObject.value("device_code").toString(),
                rootObject.value("user_code").toString(),
                rootObject.value("verification_url").toString(),
                rootObject.value("expires_in").toInt(),
                rootObject.value("interval").toInt());
    networkReply->deleteLater();
}

void OAuth2Service::tokenRequestFinished(QNetworkReply* networkReply)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(networkReply->readAll());
    QJsonObject rootObject = jsonDocument.object();
    qDebug() << "Token response:";
    qDebug() << jsonDocument.toJson();
    if(rootObject.keys().contains("error")) {
        QString error = rootObject.value("error").toString();
        QString errorDescription = rootObject.value("error_description").toString();
        emit tokenRetrieveError(error, errorDescription);
    } else {
        emit accessTokenReceived(
                    rootObject.value("access_token").toString(),
                    rootObject.value("token_type").toString(),
                    rootObject.value("expires_in").toInt(),
                    rootObject.value("refresh_token").toString());
    }
    networkReply->deleteLater();
}

GoogleOAuth2Service::GoogleOAuth2Service(QString clientId,
                                         QString clientSecret,
                                         QString scope,
                                         QObject *parent) :
    OAuth2Service("https://accounts.google.com/o/oauth2/auth",
                  "https://accounts.google.com/o/oauth2/token",
                  clientId,
                  clientSecret,
                  scope,
                  parent)
{
    //https://accounts.google.com/o/oauth2/device/code"
}

GoogleOAuth2Service::~GoogleOAuth2Service()
{
}
