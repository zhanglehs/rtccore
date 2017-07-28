#ifndef TESTSERVER_H_
#define TESTSERVER_H_
//edited by gxh
#if defined(_WIN32)
#else
//#include <unistd.h>
//#include <stdint.h>
#endif
#include "webrtc/avengine/source/gxhlog.h"
#include "webrtc/base/utility.h"
//#include "talk/examples/peerconnection/server/utility.h"//gxh
#include "webrtc/base/memberqueue.h"//gxh
#include "webrtc/base/sessionqueue.h"//gxh
#include "webrtc/base/nodequeue.h"//gxh
//#include "webrtc/base/transqueue.h"//gxh
#include "webrtc/base/sessionmanager.h"//gxh
#include "webrtc/base/nodeconn.h"//gxh
#include "webrtc/base/usermanager.h"//gxh
#include "webrtc/base/avcodec.h"//gxh
#include "webrtc/base/feccodec.h"//gxh
#include "webrtc/avengine/interface/avengAPI.h"

static int s_room_id = 0;
static char pConst[kMaxMember][kIPV4EncryptSize << 1] = {};
static char pConst2[kMaxMember][kIPV4Size] = {};
//static UserManager *g_user = NULL;

class testServer : public sigslot::has_slots<>
{
public:
	testServer(rtc::Thread *pthMain, char *server_ip, int cmd)
		:_udp_server(new ConnCore(pthMain, server_ip)), _cmd(cmd)
		, _critsect(webrtc::CriticalSectionWrapper::CreateCriticalSection())
		, _status_critsect(webrtc::CriticalSectionWrapper::CreateCriticalSection())
		, _status_event(webrtc::EventWrapper::Create())
		, _notify_critsect(webrtc::CriticalSectionWrapper::CreateCriticalSection())
		, _notify_event(webrtc::EventWrapper::Create())
		, _outroom_critsect(webrtc::CriticalSectionWrapper::CreateCriticalSection())
		, _outroom_event(webrtc::EventWrapper::Create())
		, _idx(0), _test_cnt(0)//_room_id(-1),
		, _user_queue(NULL)
		, _member_queue(NULL)
	{
		_snd_critsect = _udp_server->_snd_critsect;
		_snd_event = _udp_server->_event;
		void *pNull = NULL;
		_node_queue = new NodeQueue((BaseQueue *)pNull, _udp_server->_snd_critsect, _udp_server->_event);
		_room_queue = new SessionQueue;
		_udp_server->SignalOutPktEvent.connect(this, &testServer::OnOutPktEvent);
		_node_queue->SignalSndPktEvent0.connect(this, &testServer::OnSndPktEvent0);
		//
		_member_queue = new MemberQueue;
		
		//
		if (_cmd == kAsignCmd)
		{
			_user_queue = new UserManager(2048); //new MemberQueue;
			//_user_queue = g_user;//test
			int testsize = 0;
			if (_member_queue)
			{
				RegUserInfo *user = new RegUserInfo;
				memset(user, 0, sizeof(RegUserInfo));
				int size = sizeof(RegUserInfo);
				size >>= 2;
				int user_id = _user_queue->AddRegUser((void *)user, 0, size);
				user = (RegUserInfo *)_user_queue->GetMember(user_id);
				user->fix.user_id = user_id;
				user->fix.is_reg_user = 1;
				//
				_member_queue->Init(1, 1000);
				int member_idx = 0;
				member_idx = _member_queue->AddMember((void *)&user_id, 0, 1);		_idx++;	//rsv for ss0
				user->act.active_id = member_idx;
				testsize = _member_queue->_add->GetMaxIdx();
				//
				user = new RegUserInfo;
				memset(user, 0, sizeof(RegUserInfo));
				size = sizeof(RegUserInfo) >> 2;
				user_id = _user_queue->AddRegUser((void *)user, 0, size);
				user = (RegUserInfo *)_user_queue->GetMember(user_id);
				user->fix.user_id = user_id;
				user->fix.is_reg_user = 1;
				member_idx = _member_queue->AddMember((void *)&user_id, 0, 1);		_idx++;	//rsv for ss1
				user->act.active_id = member_idx;
				testsize = _member_queue->_add->GetMaxIdx();
			}
			//
			_status_thread = webrtc::ThreadWrapper::CreateThread(StatusRun, this, "status_manager");
			_status_thread->Start();// (tId);
			_status_thread->SetPriority(webrtc::kNormalPriority);
			//
			_notify_thread = webrtc::ThreadWrapper::CreateThread(NotifyRun, this, "notify_manager");
			_notify_thread->Start();// (tId);
			_notify_thread->SetPriority(webrtc::kNormalPriority);
			//
			_outroom_thread = webrtc::ThreadWrapper::CreateThread(OutRoomRun, this, "outroom_manager");
			_outroom_thread->Start();// (tId);
			_outroom_thread->SetPriority(webrtc::kNormalPriority);
			
		}

	}
	~testServer()
	{
		if (_status_thread.get())
		{
			if (_status_thread->Stop())
			{
				_status_thread.reset();
				_status_thread = NULL;
			}
		}
		if (_notify_thread.get())
		{
			if (_notify_thread->Stop())
			{
				_notify_thread.reset();
				_notify_thread = NULL;
			}
		}
		for (std::list<int*>::iterator it = _statusRoomList.begin(); it != _statusRoomList.end(); ++it)
		{
			delete *it;
		}
		_statusRoomList.clear();
		//
		for (std::list<int*>::iterator it = _roomList.begin(); it != _roomList.end(); ++it)
		{
			delete *it;
		}
		_roomList.clear();
		//
		for (std::list<int*>::iterator it = _outRoomList.begin(); it != _outRoomList.end(); ++it)
		{
			delete *it;
		}
		_outRoomList.clear();
		//
		for (std::list<int*>::iterator it = _memberList.begin(); it != _memberList.end(); ++it)
		{
			delete *it;
		}
		_memberList.clear();
		//
		if (_member_queue)
		{
			delete _member_queue;
		}
		if (_user_queue)
		{
			delete _user_queue;
		}
		if (_node_queue)
		{
			delete _node_queue;
		}
		if (_node_queue)
		{
			delete _node_queue;
		}
		//
		if (_status_event)
		{
			delete _status_event;
			_status_event = NULL;
		}
		if (_status_critsect)
		{
			delete _status_critsect;
		}
		if (_notify_event)
		{
			delete _notify_event;
			_notify_event = NULL;
		}
		if (_notify_critsect)
		{
			delete _notify_critsect;
		}
		if (_critsect)
		{
			delete _critsect;
		}

	}
	void PushRoom(int room_id)
	{
		_notify_critsect->Enter();
		int *uId = new int;
		*uId = room_id;
		_roomList.push_back(uId);
		_notify_critsect->Leave();
		_notify_event->Set();
	}
	void PushOutRoom(int room_id)
	{
		_outroom_critsect->Enter();
		int *uId = new int;
		*uId = room_id;
		_outRoomList.push_back(uId);
		_outroom_critsect->Leave();
		_outroom_event->Set();
	}
	void PushMember(int active_id)
	{
		_notify_critsect->Enter();
		int *uId = new int;
		*uId = active_id;
		_memberList.push_back(uId);
		_notify_critsect->Leave();
		_notify_event->Set();
	}
	void PushStatus(int room_id)
	{
		_status_critsect->Enter();
		int *uId = new int;
		*uId = room_id;
		_statusRoomList.push_back(uId);
		_status_critsect->Leave();
	}
	int PopStatus()
	{
		int ret = -1;
		_status_critsect->Enter();
		if (!_statusRoomList.empty())
		{
			int *uId = NULL;
			// Take first packet in queue
			uId = _statusRoomList.front();
			_statusRoomList.pop_front();
			ret = *uId;
			delete uId;
			uId = NULL;
		}
		_status_critsect->Leave();
		return ret;
	}
	void RepairNode()
	{
		int room_id = PopStatus();
		if (room_id >= 0)
		{
			CoreQueue *some_room = _room_queue->GetMember(room_id);
			int some_room_id = some_room->_add->GetMaxIdx();
			IdInfo *pThisId = (IdInfo *)some_room->_add->GetId(some_room_id);
			char *byeList = some_room->PopBye();
			if (byeList)
			{
				RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(pThisId->header.user_id);
				IdInfo *pOldId = (IdInfo *)byeList;
				int oldMemberType = pOldId->MemberType;
				if (user)
				{
					if (!(oldMemberType & user->act.member_type))
					{
						byeList = NULL;
					}
				}
				else
				{
				}
			}
			if (byeList)
			{
				IdInfo *pOldId = (IdInfo *)byeList;
				TransCmdHeader *headerCmd = (TransCmdHeader *)byeList;
				int ret = _node_queue->RenewNode(pOldId, some_room, some_room_id, pThisId->IdInGlob);
				if (ret > 0)
				{
					_node_queue->PushNode(some_room, some_room_id, pThisId->IdInGlob, &_udp_server->_sndPkt);
				}
				delete byeList;
			}
		}
	}
	static bool StatusRun(void* object)
	{
		return static_cast<testServer*>(object)->StatusProcess();
	}
	bool StatusProcess()
	{
		_status_event->Wait(40);

		_critsect->Enter();
		RepairNode();
		_critsect->Leave();

		return true;
	}
	void NotifyNewSession(int cmd, CoreQueue *some_room, int room_id, RegUserInfo *user, int status)
	{
#if 1
		char buf[1500] = "";
		size_t size = 0;
		TransCmdHeader *header = (TransCmdHeader *)buf;
		header->data_type = kCmd;
		header->cmd_type = cmd;// kSessionNotify;// kSessionAck;
		header->activeId = user->act.active_id;
		header->room_id = room_id;
		header->user_id = user->fix.user_id;
		header->MemberType = user->act.member_type;
		header->role_type = kServer;
		header->status = status;//kInRoom
		size = sizeof(TransCmdHeader);
		memcpy((void *)&buf[size], &some_room->_info, sizeof(SessionInfo));
		/*int testsize = sizeof(SessionInfo);
		SessionInfo *testinfo = &some_room->_info;*/
		size += sizeof(SessionInfo);
		rtc::SocketAddress dest;
		if (dest.FromString(user->act.cmdAddr))
		{
#ifndef FAST_TRANS 
			rtc::TestClient2::Packet* newPkt = new rtc::TestClient2::Packet(dest, NULL, (char *)buf, size);
			_udp_server->_snd_critsect->Enter();
			_udp_server->_sndPkt.push_back(newPkt);
			_udp_server->_snd_critsect->Leave();
			_udp_server->_event->Set();
#else
			_udp_server->SndTo(buf, size, dest);
#endif
		}
#endif
	}
	static bool NotifyRun(void* object)
	{
		return static_cast<testServer*>(object)->NotifyProcess();
	}
	bool NotifyProcess()
	{
		_notify_event->Wait(40);
		//int member_idx = _member_queue->AddMember((void *)&_idx, 0, 1);
		//notify all room info to new member
		_notify_critsect->Enter();
		while (!_memberList.empty())
		{
			int *uId = NULL;
			// Take first packet in queue
			uId = _memberList.front();
			_memberList.pop_front();
			_notify_critsect->Leave();
			int active_id = *uId;
			int size = _room_queue->_add.size();
			
			for (int i = 0; i < size; i++)
			{
				int room_id = i;
				CoreQueue *some_room = _room_queue->GetMember(room_id);
				if (some_room->_roomId >= 0)
				{
					void *userId = _member_queue->GetMember(active_id);
					int user_id = *(int *)userId;
					if (user_id == some_room->_userId)
					{
						continue;
					}
					RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(user_id);
					NotifyNewSession(kSessionNotify, some_room, room_id, user, kInRoom);
				}
			}
			//
			delete uId;
			uId = NULL;
			_notify_critsect->Enter();
		}
		_notify_critsect->Leave();
		//notify new room info to all member
		_notify_critsect->Enter();
		while (!_roomList.empty())
		{
			int *uId = NULL;
			// Take first packet in queue
			uId = _roomList.front();
			_roomList.pop_front();
			_notify_critsect->Leave();
			int room_id = *uId;
			CoreQueue *some_room = _room_queue->GetMember(room_id);
			int size = _member_queue->_add->GetMaxIdx() + 1;
			for (int i = 0; i < size; i++)
			{
				int active_id = i;
				void *idx = _member_queue->GetMember(i);
				int user_id = *(int *)idx;
				if (user_id >= 2 )
				{
					if (user_id == some_room->_userId)
					{
						continue;
					}
					RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(user_id);
					NotifyNewSession(kSessionNotify, some_room, room_id, user, kInRoom);
				}
			}
			//
			delete uId;
			uId = NULL;
			_notify_critsect->Enter();
		}
		_notify_critsect->Leave();

		return true;
	}
	//
	static bool OutRoomRun(void* object)
	{
		return static_cast<testServer*>(object)->OutRoomProcess();
	}
	bool OutRoomProcess()
	{
		_outroom_event->Wait(40);
		//notify new room info to all member
		_outroom_critsect->Enter();
		while (!_outRoomList.empty())
		{
			int *uId = NULL;
			// Take first packet in queue
			uId = _outRoomList.front();
			_outRoomList.pop_front();
			_outroom_critsect->Leave();
			int room_id = *uId;
			CoreQueue *some_room = _room_queue->GetMember(room_id);
			int size = _member_queue->_add->GetMaxIdx() + 1;
			for (int i = 0; i < size; i++)
			{
				int active_id = i;
				void *idx = _member_queue->GetMember(i);
				int user_id = *(int *)idx;
				if (user_id >= 2)
				{
					if (user_id == some_room->_userId)
					{
						continue;
					}
					RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(user_id);
					NotifyNewSession(kStatusCmd, some_room, room_id, user, kOutRoom);
				}
			}
			//
			delete uId;
			uId = NULL;
			_outroom_critsect->Enter();
		}
		_outroom_critsect->Leave();

		return true;
	}
	//
	int GetNode(TransCmdHeader *header0, const rtc::SocketAddress &dest)
	{
		int ret = 0;
		long long time0 = IGetTime();
		//int id = idx;// _idx;// 2;
		CreateSessionInfo *createHeader = (CreateSessionInfo *)header0;
		RegUserInfo *user = NULL;
		int user_id = header0->user_id;
		int active_id = header0->activeId;
		int room_id = header0->room_id;
		int speaker_id = header0->speaker_id;
		int member_type = header0->MemberType;// 0;
		RegUserInfo *thisUser = (RegUserInfo *)_user_queue->GetMember(user_id);
		/*if (!g_user)
		{
			g_user = new UserManager(2048);
		}*/

		//if (id == 2)
		if (header0->MemberType & kCreater)
		{
			room_id = _room_queue->AddMember(s_room_id);	s_room_id++;
		}

		CoreQueue *some_room = _room_queue->GetMember(room_id);
		
		if (header0->MemberType & kCreater)
		{
			int WithFEC[4] = { 0, 0, 0, 0 };
			WithFEC[0] = createHeader->info.VideoRate;
			WithFEC[1] = createHeader->info.VideoSeries;
			WithFEC[2] = createHeader->info.AudioRate;
			WithFEC[3] = createHeader->info.AudioSeries;

			some_room->Init(room_id, active_id, user_id
							, createHeader->info.RoomName
							, createHeader->info.NumOfNodeUnit
							, createHeader->info.NumOfSpeaker
							, createHeader->info.NumOfVideoLayer
							, createHeader->info.WithSSL
							, WithFEC);//createHeader->info.WithFEC);
			//some_room->Init(room_id, 4, 4, 1, 0, 0);
///			g_room = &some_room->_info;
			PushRoom(room_id);
			//
			char *data = (char *)createHeader;
			size_t size = sizeof(CreateSessionInfo);
			int *pSpeakerUserId = (int *)&data[size];
			int num = createHeader->info.NumOfNodeUnit - 1;
			//
			speaker_id = 0;
			member_type = kCreater;
			//thisUser = (RegUserInfo *)_user_queue->GetMember(user_id);
			if (thisUser)
			{
				thisUser->fix.user_id = user_id;
				thisUser->act.active_id = active_id;
				thisUser->act.member_type = kCreater;
				thisUser->act.room_id = room_id;
			}

			for (int i = 0; i < num; i++)
			{
				int speaker_user_id = pSpeakerUserId[i];
				if (speaker_user_id >= 0)
				{
					int *speakerUserId = new int;
					*speakerUserId = speaker_user_id;
					some_room->_speakerUserId.push_back(speakerUserId);
#if 0
					//some_room->InsertSpeaker(speaker_user_id);
					user = (RegUserInfo *)_user_queue->GetMember(speaker_user_id);
					if (user)
					{
						user->fix.user_id = speaker_user_id;
						//user->act.active_id = 0;
						user->act.member_type = kSpeaker;
						user->act.room_id = room_id;
					}
#endif
				}
			}
		}
		else
		{
			int num = some_room->_speakerUserId.size();
			for (int i = 0; i < num; i++)
			{
				if (user_id == *some_room->_speakerUserId[i])
				{
					member_type = kSpeaker;
					break;
				}
			}
		}
		//
		int offset = sizeof(TransCmdHeader) >> 2;
		int some_room_id = some_room->AddMember(&active_id, offset, 1);
		_room_queue->_roomMap->RenewMap(active_id, &room_id);
		///_room_queue->_roomMap->SetMap(id, &room_id);
		
		//int a = 20, b = 23;
		int a = 3, b = 6;
		IdInfo *pThisId = (IdInfo *)some_room->_add->GetId(some_room_id);

		//user = (RegUserInfo *)_user_queue->GetMember(user_id);
		if (thisUser)
		{
			thisUser->fix.user_id = user_id;
			thisUser->act.active_id = active_id;
			//user->act.member_type = kSpeaker;
			thisUser->act.room_id = room_id;

			if (member_type & kCreater || member_type & kSpeaker)
			{
				//member_type |= kCreater;
				speaker_id = some_room->GetSpeakerId();
				pThisId->IdInSon = speaker_id;// some_room->GetSpeakerId();
				some_room->AddSpeaker(1, active_id, pThisId->IdInSon);
				//some_room->AddSpeaker(1);

			}
			else
			{
				thisUser->act.member_type = kClient;
				/*if (active_id == (kMaxMember - 1))
				{
					printf("");
				}*/
				//member_type |= kClient;
			}
		}
		else
		{
			member_type |= kClient;
		}
		/*offset += 3;
		some_room->SetMember(some_room_id, (void *)&member_type, offset, 1);
		offset += 3;
		some_room->SetMember(some_room_id, (void *)pConst[id], offset, (kIPV4EncryptSize << 1) >> 2);
		offset += (kIPV4EncryptSize << 1) >> 2;
		some_room->SetMember(some_room_id, (void *)pConst2[id], offset, kIPV4Size >> 2);*/
		//
		char buf[1500] = "";
		size_t size = 0;
		TransCmdHeader *header = (TransCmdHeader *)buf;
		header->data_type = kCmd;
		header->cmd_type = kSessionAck;//speakerId
		header->activeId = active_id;
		header->room_id = room_id;
		header->user_id = user_id;
		header->speaker_id = speaker_id;
		header->MemberType = member_type;// thisUser->act.member_type;// header0->MemberType;
		size = sizeof(TransCmdHeader);
		memcpy((void *)&buf[size], &some_room->_info, sizeof(SessionInfo));
		/*int testsize = sizeof(SessionInfo);
		SessionInfo *testinfo = &some_room->_info;*/
		size += sizeof(SessionInfo);
#ifndef FAST_TRANS 
		rtc::TestClient2::Packet* newPkt = new rtc::TestClient2::Packet(dest, NULL, (char *)buf, size);
		_udp_server->_snd_critsect->Enter();
		_udp_server->_sndPkt.push_back(newPkt);
		_udp_server->_snd_critsect->Leave();
		_udp_server->_event->Set();
#else
		_udp_server->SndTo(buf, size, dest);
#endif
		//
		pThisId->MemberType = member_type;
		memcpy(pThisId->uIp, pConst[active_id], (kIPV4EncryptSize << 1));
		strcpy(pThisId->addr, pConst2[active_id]);
		pThisId->IdInGlob = active_id;
		pThisId->IdInSession = some_room_id;
		pThisId->header.user_id = user_id;
		pThisId->header.room_id = room_id;;
		//
		//if (id != 2)
		if (!(thisUser->act.member_type & kCreater))
		{
			char *byeList = some_room->PopBye();
			if (byeList)
			{
				IdInfo *pOldId = (IdInfo *)byeList;
				int oldMemberType = pOldId->MemberType;
				if (!(oldMemberType & thisUser->act.member_type))
				{
					byeList = NULL;
				}
			}
			if (byeList)
			{
				IdInfo *pOldId = (IdInfo *)byeList;
				TransCmdHeader *headerCmd = (TransCmdHeader *)byeList;
				ret = _node_queue->RenewNode(pOldId, some_room, some_room_id, active_id);
				delete byeList;
				printf("byeList \n");
			}
			else
			{
				ret = _node_queue->GetStunNodeInfo(some_room, some_room_id, active_id);
			}
			if (ret > 0)
			{
				//_udp_server->_snd_critsect->Enter();
				_node_queue->PushNode(some_room, some_room_id, active_id, &_udp_server->_sndPkt);
				//_udp_server->_snd_critsect->Leave();
				//printf("######################### _test_cnt = %d: id = %d ret = %d \n", _test_cnt, id, ret);
			}
		}
		else
		{
			_node_queue->PushNode(some_room, some_room_id, active_id, &_udp_server->_sndPkt);
		}
		_test_cnt++;
		long long time1 = IGetTime();
		int difftime = (int)(time1 - time0);
///		printf("difftime = %d \n", difftime);
		//
#if 0
		if (_idx == (kMaxMember - 1))
		{

			some_room_id = 1;
			for (id = 3; id < kMaxMember; id++)
			{
				some_room = _room_queue->GetMember(_room_id);
				std::list<rtc::TestClient2::Packet *> nodeContext;
				int ret = _node_queue->GetStunNodeInfo(some_room, some_room_id, id, nodeContext);
				if (ret > 0)
				{
					printf("$$$$$$$$$$$$$$$$$$$$$$$$$$ id = %d ret = %d \n", id, ret);
				}

				if (id != (kMaxMember - 1))
				{
					some_room_id++;
					continue;
				}
				SaveNodeInfo(_jsonHnd, some_room, some_room_id, id);
				some_room_id++;
			}
		}
#endif
		return ret;
	}
	void OnOutPktEvent(rtc::TestClient2::Packet* pkt)
	{
        printf("OnOutPktEvent \n");
		const rtc::SocketAddress dest(pkt->addr);
		std::string sRemote = dest.HostAsURIString();//GXH_TEST
		uint16 iPort = dest.port();
		if (_cmd == kStunCmd)
		{
			TransCmdHeader *header = (TransCmdHeader *)pkt->buf;
			if (header->data_type == kCmd && header->cmd_type == kStunCmd)
			{
				char buf[1500] = "";
				size_t size = 0;
				int activeId = header->activeId;
				header = (TransCmdHeader *)buf;
				header->data_type = kCmd;
				header->cmd_type = kStunAck;// _cmd;
				//header->activeId = _idx;
				header->activeId = activeId;
				//header->user_id =
				size = sizeof(TransCmdHeader);
				char *p = (char *)&buf[size];
				//					
				unsigned long long iaddr = 0;
				char src[kIPV4Size] = "";
				strcpy(src, sRemote.c_str());
				strcat(src, ":");
				sprintf(&src[strlen(src)], "%d", iPort);
				strcpy(pConst2[activeId], src);
				ipv4toUint64(&iaddr, (char *)src);

				memcpy(pConst[activeId], &iaddr, kIPV4EncryptSize);
				memcpy(&pConst[activeId][kIPV4EncryptSize], &iaddr, kIPV4EncryptSize);
				//test
				char dst[kIPV4Size] = "";
				int ret = uint64toIpv4((unsigned long long *)pConst[activeId], (char *)dst);
				printf("kStunCmd: %d, id0 = %d: addr = %s:%d \n", _idx, activeId, sRemote.c_str(), iPort);//is ok
				if (activeId != _idx)
				{
					printf("error \n");
				}
				memcpy(p, &iaddr, kIPV4EncryptSize);
				memcpy(&p[kIPV4EncryptSize], &iaddr, kIPV4EncryptSize);
				memcpy(&p[kIPV4EncryptSize << 1], src, kIPV4Size);
				size += ((kIPV4EncryptSize << 1) + kIPV4Size) * sizeof(char);
#ifndef FAST_TRANS 
				rtc::TestClient2::Packet* newPkt = new rtc::TestClient2::Packet(dest, NULL, (char *)buf, size);
				_udp_server->_snd_critsect->Enter();
				_udp_server->_sndPkt.push_back(newPkt);
				_udp_server->_snd_critsect->Leave();
				_udp_server->_event->Set();
#else
				_udp_server->SndTo(buf, size, dest);
#endif
				
				_idx++;
			}
		}
		else if (_cmd == kAsignCmd)//kSessionCmd)
		{
            printf("OnOutPktEvent: kAsignCmd \n");
			_critsect->Enter();
			TransCmdHeader *header = (TransCmdHeader *)pkt->buf;
			int activeId = header->activeId;
			if (header->data_type == kCmd)
			{
				if (header->cmd_type == kRegCmd)
				{
					int  active_id = header->activeId;
					void *userId = _member_queue->GetMember(active_id);
					int user_id = *(int *)userId;
					RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(user_id);
					user->fix.is_reg_user = 1;
					//user->act.active_id = activeId;
					//user->fix.user_name;
					//test
					//strcpy(user->act.cAddr, pConst2[activeId]);
					//memcpy(user->act.uAddr, pConst[activeId], kIPV4EncryptSize);//cmdAddr
					//PushMember(active_id);//test
					
				}
				else if (header->cmd_type == kActiveCmd)
				{
                    //printf("OnOutPktEvent: kAsignCmd: kActiveCmd \n");
					RegUserInfo *user = NULL;
					int active_id = header->activeId;
					if (active_id < 0)
					{
						RegisterInfo *regInfor = (RegisterInfo *)&pkt->buf[sizeof(TransCmdHeader)];
						int user_id = header->user_id;
						if (user_id < 0)
						{
                            //printf("OnOutPktEvent: kAsignCmd: kActiveCmd: 0 \n");
							user = new RegUserInfo;
							memset(user, 0, sizeof(RegUserInfo));
							int size = sizeof(RegUserInfo) >> 2;
							user_id = _user_queue->AddRegUser((void *)user, 0, size);
							user = (RegUserInfo *)_user_queue->GetMember(user_id);
							strcpy(user->fix.user_name, regInfor->user_name);
							user->fix.user_id = user_id;
							user->fix.is_reg_user = 0;
							user->act.active_id;
							user->act.member_type;
							user->act.room_id;
							char src[kIPV4Size] = "";
							strcpy(src, sRemote.c_str());
							strcat(src, ":");
							sprintf(&src[strlen(src)], "%d", iPort);
							strcpy(user->act.cmdAddr, src);
						}
#if 1
						else
						{
							user = (RegUserInfo *)_user_queue->GetMember(user_id);
							strcpy(user->fix.user_name, regInfor->user_name);
							user->fix.user_id = user_id;
							user->fix.is_reg_user = 0;
							user->act.active_id;
							user->act.member_type;
							user->act.room_id;
							char src[kIPV4Size] = "";
							strcpy(src, sRemote.c_str());
							strcat(src, ":");
							sprintf(&src[strlen(src)], "%d", iPort);
							strcpy(user->act.cmdAddr, src);
						}
#endif
                        //printf("OnOutPktEvent: kAsignCmd: kActiveCmd: 1 \n");
						//int member_idx = _member_queue->AddMember((void *)&_idx, 0, 1);
						int member_idx = _member_queue->AddMember((void *)&user_id, 0, 1);
						user->act.active_id = member_idx;
						PushMember(member_idx);//?
                        //printf("OnOutPktEvent: kAsignCmd: kActiveCmd: 2 \n");
						//
						char buf[1500] = "";
						size_t size = 0;
						int id0 = header->activeId;
						header = (TransCmdHeader *)buf;
						header->data_type = kCmd;
						header->cmd_type = kActiveAck;// _cmd;
						header->activeId = member_idx;// _idx;//
						header->user_id = user_id;
						size = sizeof(TransCmdHeader);
                        
                        //printf("OnOutPktEvent: kAsignCmd: kActiveCmd: 3 \n");
                        //printf("server:kActiveCmd _idx = %d: activeId=%d addr = %s:%d \n", _idx, activeId, sRemote.c_str(), iPort);

#ifndef FAST_TRANS
						rtc::TestClient2::Packet* newPkt = new rtc::TestClient2::Packet(dest, NULL, (char *)buf, size);
						_udp_server->_snd_critsect->Enter();
						_udp_server->_sndPkt.push_back(newPkt);
						_udp_server->_snd_critsect->Leave();
						_udp_server->_event->Set();
#else
						_udp_server->SndTo(buf, size, dest);
#endif

						printf("server:kActiveCmd _idx = %d: activeId=%d addr = %s:%d \n", _idx, activeId, sRemote.c_str(), iPort);
						_idx++;
					}
				}
				else if (header->cmd_type == kSessionCmd)
				{
					//test
					int status = header->status;
					if (status == kInRoom)
					{
						int  active_id = header->activeId;
						void *userId = _member_queue->GetMember(active_id);
						int user_id = *(int *)userId;
						RegUserInfo *user = (RegUserInfo *)_user_queue->GetMember(user_id);
						strcpy(user->act.cAddr, pConst2[activeId]);
						memcpy(user->act.uAddr, pConst[activeId], kIPV4EncryptSize);//cmdAddr
						//
						GetNode(header, dest);
						printf("server:kSessionCmd _idx = %d: activeId=%d addr = %s:%d \n", _idx, activeId, sRemote.c_str(), iPort);
					}
					else
					{
						int active_id = header->activeId;
						int user_id = header->user_id;
					}
					//_idx++;
				}
				else if (header->cmd_type == kAsignAck)
				{
					///printf("server:kAsignAck %d: addr = %s:%d \n", activeId, sRemote.c_str(), iPort);
				}
				else if (header->cmd_type == kStatusCmd)
				{
					IdInfo *pSelfId = (IdInfo *)pkt->buf;
					TransCmdHeader *headerCmd = (TransCmdHeader *)pkt->buf;
					int room_id = headerCmd->room_id;
					int active_id = headerCmd->activeId;
					int MemberType = headerCmd->MemberType;
					if (MemberType & kCreater)
					{
						CoreQueue *some_room = _room_queue->GetMember(room_id);
						some_room->Clear();
						_room_queue->SubMember(room_id);
						//add here
						//notify all member to close room
						PushOutRoom(room_id);
						printf("close room: %d \n", room_id);
						//
						//CoreQueue *some_room = _room_queue->GetMember(room_id);
						//some_room->SubMember(active_id);
						some_room->PushBye(pkt->buf, pkt->size);
						_status_event->Set();
						PushStatus(room_id);
						printf("server:kStatusCmd %d: addr = %s:%d \n", activeId, sRemote.c_str(), iPort);
					}
					else
					{
						CoreQueue *some_room = _room_queue->GetMember(room_id);
						int ret = some_room->GetMemberNum();
						if (ret > 0)
						{
							some_room->SubMember(active_id);
							some_room->PushBye(pkt->buf, pkt->size);
							_status_event->Set();
							PushStatus(room_id);
						}
						printf("server:kStatusCmd %d: addr = %s:%d \n", activeId, sRemote.c_str(), iPort);
					}
				}
			}
			_critsect->Leave();

		}
		//delete pkt;
		//pkt = NULL;
		//_idx++;
	}
	void OnSndPktEvent0(rtc::TestClient2::Packet* pkt)
	{
		if (pkt)
		{
			char* buf = pkt->buf;
			size_t size = pkt->size;
			const rtc::SocketAddress dest = pkt->addr;
			_udp_server->SndTo(buf, size, dest);
			delete pkt;
		}
	}
public:
	ConnCore *_udp_server;
	rtc::scoped_ptr<webrtc::ThreadWrapper> _status_thread;
	webrtc::CriticalSectionWrapper* _status_critsect;
	webrtc::EventWrapper *_status_event;

	rtc::scoped_ptr<webrtc::ThreadWrapper> _notify_thread;
	webrtc::CriticalSectionWrapper* _notify_critsect;
	webrtc::EventWrapper *_notify_event;

	rtc::scoped_ptr<webrtc::ThreadWrapper> _outroom_thread;
	webrtc::CriticalSectionWrapper* _outroom_critsect;
	webrtc::EventWrapper *_outroom_event;
	

	webrtc::CriticalSectionWrapper* _critsect;
	webrtc::CriticalSectionWrapper* _snd_critsect;
	webrtc::EventWrapper *_snd_event;
	NodeQueue *_node_queue;
	SessionQueue *_room_queue;
	MemberQueue *_member_queue;
	UserManager *_user_queue;
	//MemberQueue *_user_queue;//test
	std::list<int*> _statusRoomList;
	std::list<int*> _roomList;
	std::list<int*> _outRoomList;
	std::list<int*> _memberList;
	int _idx;
	int _cmd;
	//int _room_id;
	int _test_cnt;
};
#endif