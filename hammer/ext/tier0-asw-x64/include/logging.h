#pragma once

// Sourced from the Alien Swarm SDK

//============ Copyright (c) Valve Corporation, All rights reserved. ============
//
// Logging system declarations.
//
// The logging system is a channel-based output mechanism which allows
// subsystems to route their text/diagnostic output to various listeners
//
//===============================================================================


//////////////////////////////////////////////////////////////////////////
// Logging Macros
//////////////////////////////////////////////////////////////////////////

// This macro will resolve to the most appropriate overload of LoggingSystem_Log() depending on the number of parameters passed in.
#define InternalMsg( Channel, Severity, /* [Color], Message, */ ... ) do { if ( LoggingSystem_IsChannelEnabled( Channel, Severity ) ) LoggingSystem_Log( Channel, Severity, /* [Color], Message, */ ##__VA_ARGS__ ); } while( 0 )

//-----------------------------------------------------------------------------
// New macros, use these!
//
// The macros take an optional Color parameter followed by the message 
// and the message formatting.
// We rely on the variadic macro (__VA_ARGS__) operator to paste in the 
// extra parameters and resolve to the appropriate overload.
//-----------------------------------------------------------------------------
#define Log_Msg( Channel, /* [Color], Message, */ ... ) InternalMsg( Channel, LS_MESSAGE, /* [Color], Message, */ ##__VA_ARGS__ )
#define Log_Warning( Channel, /* [Color], Message, */ ... ) InternalMsg( Channel, LS_WARNING, /* [Color], Message, */ ##__VA_ARGS__ )
#define Log_Error( Channel, /* [Color], Message, */ ... ) InternalMsg( Channel, LS_ERROR, /* [Color], Message, */ ##__VA_ARGS__ )
#define Log_Assert( Message, ... ) LoggingSystem_LogAssert( Message, ##__VA_ARGS__ )

//////////////////////////////////////////////////////////////////////////
// DLL Exports
//////////////////////////////////////////////////////////////////////////

#define FMTFUNCTION( a, b )

typedef int LoggingChannelID_t;
typedef int ILoggingResponsePolicy;
typedef int ILoggingListener;
typedef void LoggingChannel_t;
typedef int LoggingSeverity_t;
typedef int LoggingChannelFlags_t;
typedef int LoggingResponse_t;
typedef int int32;

struct Color {
	unsigned char r, g, b, a;
};

extern "C" {

void LoggingSystem_RegisterLoggingListener( ILoggingListener *pListener );
void LoggingSystem_ResetCurrentLoggingState();
void LoggingSystem_SetLoggingResponsePolicy( ILoggingResponsePolicy *pResponsePolicy );
// NOTE: PushLoggingState() saves the current logging state on a stack and results in a new clear state
// (no listeners, default logging response policy).
void LoggingSystem_PushLoggingState( bool bThreadLocal, bool bClearState );
void LoggingSystem_PopLoggingState( bool bThreadLocal );

void LoggingSystem_AddTagToCurrentChannel( const char *pTagName );

// Returns INVALID_LOGGING_CHANNEL_ID if not found
LoggingChannelID_t LoggingSystem_FindChannel( const char *pChannelName );
int LoggingSystem_GetChannelCount();
LoggingChannelID_t LoggingSystem_GetFirstChannelID();
// Returns INVALID_LOGGING_CHANNEL_ID when there are no channels remaining.
LoggingChannelID_t LoggingSystem_GetNextChannelID( LoggingChannelID_t channelID );
const LoggingChannel_t *LoggingSystem_GetChannel( LoggingChannelID_t channelID );

bool LoggingSystem_HasTag( LoggingChannelID_t channelID, const char *pTag );

bool LoggingSystem_IsChannelEnabled( LoggingChannelID_t channelID, LoggingSeverity_t severity );
void LoggingSystem_SetChannelSpewLevel( LoggingChannelID_t channelID, LoggingSeverity_t minimumSeverity );
void LoggingSystem_SetChannelSpewLevelByName( const char *pName, LoggingSeverity_t minimumSeverity );
void LoggingSystem_SetChannelSpewLevelByTag( const char *pTag, LoggingSeverity_t minimumSeverity );

// Color is represented as an int32 due to C-linkage restrictions
int32 LoggingSystem_GetChannelColor( LoggingChannelID_t channelID );
void LoggingSystem_SetChannelColor( LoggingChannelID_t channelID, int color );

LoggingChannelFlags_t LoggingSystem_GetChannelFlags( LoggingChannelID_t channelID );
void LoggingSystem_SetChannelFlags( LoggingChannelID_t channelID, LoggingChannelFlags_t flags );

//-----------------------------------------------------------------------------
// Logs a variable-argument to a given channel with the specified severity.
// NOTE: if adding overloads to this function, remember that the Log_***
// macros simply pass their variadic parameters through to LoggingSystem_Log().
// Therefore, you need to ensure that the parameters are in the same general 
// order and that there are no ambiguities with the overload.
//-----------------------------------------------------------------------------
LoggingResponse_t LoggingSystem_Log( LoggingChannelID_t channelID, LoggingSeverity_t severity, const char *pMessageFormat, ... ) FMTFUNCTION( 3, 4 );
// LoggingResponse_t LoggingSystem_Log( LoggingChannelID_t channelID, LoggingSeverity_t severity, Color spewColor, const char *pMessageFormat, ... ) FMTFUNCTION( 4, 5 );

LoggingResponse_t LoggingSystem_LogDirect( LoggingChannelID_t channelID, LoggingSeverity_t severity, Color spewColor, const char *pMessage );
LoggingResponse_t LoggingSystem_LogAssert( const char *pMessageFormat, ... ) FMTFUNCTION( 1, 2 );

}
