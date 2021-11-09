/****************************************************************/
/*
Goomba Stomp plugin for bzflag.

Jarrett Cigainero, 2021
	MIT License.
	Explicitly: feel free to use this plugin, including with your modifications, on any server.
*/
/****************************************************************/

#include "bzfsAPI.h"
#include "plugin_utils.h"

bool playerAlive[255];
float playerPosX[255];
float playerPosY[255];
float playerPosZ[255];
float playerVertVel[255];

class GOOMBA : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
    virtual bool SlashCommand(int, bz_ApiString, bz_ApiString, bz_APIStringList*);
    virtual const char* Name ()
    {
        return "Goomba Stomp";
    }
    virtual void Init ( const char* config );
    virtual void Cleanup(void);
    virtual void Event(bz_EventData *eventData);
};

BZ_PLUGIN(GOOMBA)

void GOOMBA::Init ( const char* /*commandLine*/ )
{
    bz_debugMessage(4,"Goomba Stomp plugin loaded");

    for(int i=0;i<255;i++)playerAlive[i] = 0;

    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_ePlayerUpdateEvent);

    bz_registerCustomSlashCommand("goomba", this);
    bz_registerCustomSlashCommand("laugh", this);

    // init events here with Register();
}
void GOOMBA::Cleanup(void)
{
    Flush();
    bz_removeCustomSlashCommand("goomba");
    bz_removeCustomSlashCommand("laugh");
}

/*****************************************/
/* EVENTS HANDLER */
/*****************************************/
void GOOMBA::Event(bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerDieEvent:{
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            playerAlive[updateData->playerID] = 0;
        }
        break;
        case bz_ePlayerPartEvent:{
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            playerAlive[updateData->playerID] = 0;
        }
        break;
        case bz_ePlayerSpawnEvent:{
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            playerAlive[updateData->playerID] = 1;
        }
        break;
        case bz_ePlayerUpdateEvent:{
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            //playerAlive[updateData->playerID];

            //Update player positions and get a rough idea of positive or negative vertical movement.
            playerPosX[updateData->playerID] = updateData->state.pos[0];
            playerPosY[updateData->playerID] = updateData->state.pos[1];
            playerVertVel[updateData->playerID] = updateData->state.pos[2] - playerPosZ[updateData->playerID];  //Not actual vertical velocity. Just want to get a rough estimate.
            playerPosZ[updateData->playerID] = updateData->state.pos[2];

            for(int pv=0;pv<255;pv++){
                //Check to see if player is alive first. Don't check anyone else; it's a waste of time.
                if(playerAlive[pv]){
                    //Find someone who isn't the updated player and check if updated player is alive. Only alive players can stomp.
                    if((pv != updateData->playerID) && playerAlive[updateData->playerID]){
                        //Check for a stomp. Only check players who are in a downward motion and not anyone else; it's a waste of time.
                        if(playerVertVel[updateData->playerID] < 0){
                            //Get stomper's Position Data.
                            float stomperX = updateData->state.pos[0];
                            float stomperY = updateData->state.pos[1];
                            float stomperZ = updateData->state.pos[2];
                            //Check for player collision, and difference in vertical velocity.
                            if((playerPosX[pv] > (stomperX-8)) && (playerPosX[pv] < (stomperX+8)) &&
                               (playerPosY[pv] > (stomperY-8)) && (playerPosY[pv] < (stomperY+8)) &&
                               (playerPosZ[pv] > (stomperZ-8)) && (playerPosZ[pv] < (stomperZ+8)) &&
                               (playerVertVel[updateData->playerID] < playerVertVel[pv]))
                               {
                                   //If found, figure out who stomped who and kill the *stomped* player.
                                    bz_setPlayerWins(updateData->playerID, (bz_getPlayerWins(updateData->playerID) + 2)); //Give the stomper some extra points.
                                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s was goomba stomped by %s!",bz_getPlayerCallsign(pv),bz_getPlayerCallsign(updateData->playerID));
                                    bz_killPlayer (pv, 0, pv, "SR");
                                    bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "goomba.wav" ); //Play a funny sound if the sound exists in ./data.
                                    //bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "http://stupidnet.duckdns.org/goomba.wav" );
                                    //Reset the dead player.
                                    playerPosX[pv] = 0;
                                    playerPosY[pv] = 0;
                                    playerPosZ[pv] = -10;
                                    playerAlive[pv] = 0;
                                    playerVertVel[pv] = 0;
                               }
                        }
                    }
                }
            }
        }
        break;
        default:
        break;
    }
}

bool GOOMBA::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (command == "goomba"){
        bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "goomba.wav" );
        return true;
    }
    if (command == "laugh"){
        bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "flag_lost.wav" );
        return true;
    }
    return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
