#include <flatbufserver/FlatBufferServer.h>
#include "FlatBufferClient.h"
#include "HyperionConfig.h"

// util
#include <utils/NetOrigin.h>
#include <utils/GlobalSignals.h>

// qt
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLocalServer>

// Constants
namespace {

const char SERVICE_TYPE[] = "flatbuffer";

} //End of constants

FlatBufferServer::FlatBufferServer(const QJsonDocument& config, QObject* parent)
	: QObject(parent)
	, _server(new QTcpServer(this))
	, _domain(new QLocalServer(this))
	, _log(Logger::getInstance("FLATBUFSERVER"))
	, _timeout(5000)
	, _config(config)
{

}

FlatBufferServer::~FlatBufferServer()
{
	stopServer();
	delete _server;
	delete _domain;
}

void FlatBufferServer::initServer()
{
	_netOrigin = NetOrigin::getInstance();
	if (_server != nullptr)
		connect(_server, &QTcpServer::newConnection, this, &FlatBufferServer::newConnection);

	if (_domain != nullptr)
		connect(_domain, &QLocalServer::newConnection, this, &FlatBufferServer::newConnection);

	// apply config
	handleSettingsUpdate(settings::FLATBUFSERVER, _config);
}

void FlatBufferServer::handleSettingsUpdate(settings::type type, const QJsonDocument& config)
{
	if(type == settings::FLATBUFSERVER)
	{
		const QJsonObject& obj = config.object();

		quint16 port = obj["port"].toInt(19400);

		// port check
		if(_server != nullptr && _server->serverPort() != port)
		{
			stopServer();
			_port = port;
		}

		// new timeout just for new connections
		_timeout = obj["timeout"].toInt(5000);
		// enable check
		obj["enable"].toBool(true) ? startServer() : stopServer();
	}
}


void FlatBufferServer::setupClient(FlatBufferClient* client)
{
	connect(client, &FlatBufferClient::clientDisconnected, this, &FlatBufferServer::clientDisconnected);
	connect(client, &FlatBufferClient::registerGlobalInput, GlobalSignals::getInstance(), &GlobalSignals::registerGlobalInput);
	connect(client, &FlatBufferClient::clearGlobalInput, GlobalSignals::getInstance(), &GlobalSignals::clearGlobalInput);
	connect(client, &FlatBufferClient::setGlobalInputImage, GlobalSignals::getInstance(), &GlobalSignals::setGlobalImage);
	connect(client, &FlatBufferClient::setGlobalInputColor, GlobalSignals::getInstance(), &GlobalSignals::setGlobalColor);
	connect(client, &FlatBufferClient::setBufferImage, GlobalSignals::getInstance(), &GlobalSignals::setBufferImage);
	connect(GlobalSignals::getInstance(), &GlobalSignals::globalRegRequired, client, &FlatBufferClient::registationRequired);
	_openConnections.append(client);
}

void FlatBufferServer::newConnection()
{
	while(_server != nullptr && _server->hasPendingConnections())
	{
		if(QTcpSocket* socket = _server->nextPendingConnection())
		{
			if(_netOrigin->accessAllowed(socket->peerAddress(), socket->localAddress()))
			{
				Debug(_log, "New connection from %s", QSTRING_CSTR(socket->peerAddress().toString()));
				FlatBufferClient *client = new FlatBufferClient(socket, nullptr, _timeout, this);
				// internal
				setupClient(client);
			}
			else
				socket->close();
		}
	}
	while (_domain != nullptr && _domain->hasPendingConnections())
	{
		if (QLocalSocket* socket = _domain->nextPendingConnection())
		{
			Debug(_log, "New local domain connection");
			FlatBufferClient* client = new FlatBufferClient(nullptr, socket, _timeout, this);
			// internal
			setupClient(client);
		}
	}
}

void FlatBufferServer::clientDisconnected()
{
	FlatBufferClient* client = qobject_cast<FlatBufferClient*>(sender());
	client->deleteLater();
	_openConnections.removeAll(client);
}

void FlatBufferServer::startServer()
{
	if(_server != nullptr && !_server->isListening())
	{
		if(!_server->listen(QHostAddress::Any, _port))
		{
			Error(_log,"Failed to bind port %d", _port);
		}
		else
		{
			Info(_log,"Started on port %d", _port);

			emit publishService(SERVICE_TYPE, _port);
		}
	}
	if (_domain != nullptr && !_domain->isListening())
	{
		if (!_domain->listen(HYPERION_DOMAIN_SERVER))
			Error(_log, "Could not start local domain server 'hyperion'");
		else
			Info(_log, "Started local domain server: %s", QSTRING_CSTR(_domain->serverName()));
	}
}

void FlatBufferServer::stopServer()
{
	if ((_server != nullptr &&_server->isListening()) || (_domain != nullptr && _domain->isListening()))
	{
		// close client connections
		for(const auto& client : _openConnections)
		{
			client->forceClose();
		}

		if (_server != nullptr)
			_server->close();

		if (_domain != nullptr)
			_domain->close();

		Info(_log, "Stopped");
	}
}
