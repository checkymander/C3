#pragma once

namespace FSecure::C3::Interfaces::Channels
{
    /// Implementation of a File Channel.
    class MSSQL : public FSecure::C3::Interfaces::Channel<MSSQL>
    {
    public:
        /// Public constructor.
        /// @param arguments factory arguments.
        MSSQL(FSecure::ByteView arguments);
        /// OnSend callback implementation. Called every time attached Relay wants to send a packet through this Channel Device. @see Device::OnSendToChannelInternal.
        /// @param packet data to send through the Channel.
        /// @return number of bytes successfully sent through the Channel. One call does not have to send all data. In that case chunking will take place, Chunks must be at least 64 bytes or equal to packet.size() to be accepted,
        size_t OnSendToChannel(FSecure::ByteView packet);

        /// Reads a single C3 packet from Channel. Periodically called by attached Relay. Implementation should read the data (or return an empty buffer if there's nothing in the Channel waiting to read) and leave as soon as possible.
        /// @return ByteVector that contains a single packet retrieved from Channel.
        FSecure::ByteVector OnReceiveFromChannel();

        /// Processes Commands addressed to this Channel.
        /// @param command a buffer containing whole command and it's parameters.
        /// @return command result.
        ByteVector OnRunCommand(ByteView command) override;

        /// Example Command handler. It must be registered/described in GetCapability. @see MyChannel::OnRunCommand.
        /// @returns ByteVector Command result.
        ByteVector ClearTable();

        /// Describes Channels creation parameters and custom Commands.
        /// @return Channel's capability description in JSON format.
        static const char* GetCapability();

        /// Explicit values used as the defaults for Channel's UpdateDelayJitter. Values can be changed later, at runtime.
        constexpr static std::chrono::milliseconds s_MinUpdateDelay = 1000ms, s_MaxUpdateDelay = 1000ms;

    private:
        /// The String to append to outbound communications
        std::string m_InboundDirectionName;

        /// The String to append to inbound communications
        std::string m_OutboundDirectionName;

        /// The server name to handle communication
        std::string servername;

        /// The database name to handle communication
        std::string databasename;

        /// The table name to store all messages
        std::string tablename;

        /// The user to authenticate to the database
        std::string username;

        /// The password for the user
        std::string password;

    };
}