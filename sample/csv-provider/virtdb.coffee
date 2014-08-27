# CONST = require('./config').Const
udp         = require 'dgram'
async       = require "async"
Protocol    = require './protocol'

class VirtDB
    svcConfigSocket: null   # Communicating endpoints and config
    IP: null

    constructor: (@name, connectionString) ->
        # Connect to VirtDB Service Config
        Protocol.svcConfig connectionString, @_onEndpoint

        endpoint =
            Endpoints: [
                Name: @name
                SvcType: 'NONE'
            ]
        Protocol.sendEndpoint endpoint

    _setupEndpoint: (protocol_call, callback) =>
        protocol_call 'tcp://' + @IP + ':*', callback, (err, svcType, zmqType, zmqAddress) =>
            console.log "Listening (" + svcType + ") on", zmqAddress
            endpoint =
                Endpoints: [
                    Name: @name
                    SvcType: svcType
                    Connections: [
                        Type: zmqType
                        Address: [
                            zmqAddress
                        ]
                    ]
                ]
            Protocol.sendEndpoint endpoint


    onMetaDataRequest: (callback) =>
        @_onIP =>
            @_setupEndpoint Protocol.metaDataServer, callback
        return

    onQuery: (callback) =>
        @_onIP =>
            @_setupEndpoint Protocol.queryServer, callback
            @_setupEndpoint Protocol.columnServer

    sendMetaData: (data) ->
        Protocol.sendMetaData data

    sendColumn: (data) ->
        Protocol.sendColumn data

    close: =>
        @svcConfigScoket.close()
        @metadata_socket.close()
        @column_socket.close()


    _onIP: (callback) =>
        if @IP?
            callback()
        else
            async.retry 5, (retry_callback, results) =>
                setTimeout =>
                    err = null
                    if not @IP?
                        err "IP is not set yet"
                    retry_callback err, @IP
                , 50
            , ->
                callback()

    _onEndpoint: (endpoint) =>
        switch endpoint.SvcType
            when 'IP_DISCOVERY'
                if not @IP?
                    @_findMyIP endpoint.Connections[0].Address.toString()

    _findMyIP: (discoveryAddress) =>
        if discoveryAddress.indexOf 'raw_udp://' == 0
            client = null
            message = new Buffer('?')
            address = discoveryAddress.replace /^raw_udp:\/\//, ''
            if address.indexOf('[') > -1 # IPv6
                ip = address.replace /^\[|\]:[0-9]{2,5}/g, ''
                port = address.replace /\[.*\]:/g, ''
                client = udp.createSocket 'udp6'
            else    # IPv4
                parts = address.split(':')
                ip = parts[0]
                port = parts[1]
                client = udp.createSocket 'udp4'

            client?.on 'message', (message, remote) =>
                @IP = message.toString()
                client.close()


            async.retry 5, (callback, results) =>
                err = null
                client?.send message, 0, 1, port, ip, (err, bytes) ->
                    if err
                        console.log err
                setTimeout =>
                    if @IP == null
                        err = "IP is not set yet!"
                    callback err, @IP
                , 50
            , ->
                return


module.exports = VirtDB
