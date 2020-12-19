#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <limits>
#include <unistd.h>
#include <algorithm>
//#define nc
//#define dbg
#include <chrono>
#include <limits.h>

using namespace model;
using namespace std;
enum enm_vehs{_UNKNOWN_=-1,ARRV,FIGHTER,HELI,IFV,TANK,ALLY};
enum enm_priority{MIN=0,LOW=5,MED=10,HIGH=15,MAX=100};
enum enm_mission_type{START,ROUTE,SET_PRODUCE};
enum enm_group{NO,AR,T1,I1,F,H};
enum enm_headings{ZERO,NW=1,N,NE,E,W,SW,S,SE};
enum enm_grp_move_status{NONE,NEW_ROUTE,ONTHEWAY,DETOUR,NOWAY,STUCK,SCALING};
enum enm_action{no_act,MOVE,ROTATE,DOWNSCALE,NUKE,NUKE_EVASION,DOWNSCALE_ALL,ASSIGN};
enum enm_grp_task{no,capture,healup,hunt,follow,move2base};
enum enm_fac_status{no_status,capturing,producing,captured};
#define C_RANGE_MAX 3//collison detection
#define C_RANGE_MIN 1
#define D_STEP_STRAIGHT 64 //detour len px
#define STOP_D 400 //squared distance when detour not necessary.
#define PF_SIZE 32
#define PF_SIZE_0 31
#define SCALE_RATE 50//ticks
#define TILE_SIZE 8
#define TILES_COUNT 128
#define TILES_SQUARED 16384
#define TILES_COUNT_0 127
#define TILE_COEF 4

#ifdef nc
#include <ncurses.h>
#endif
template<typename T>
static bool areEqual(T f1, T f2) {
	return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(fabs(f1), fabs(f2)));
}
struct tile_coord
{
	int x,y;
};
inline int distTile(const tile_coord& t1,const tile_coord& t2)//const tile_coord t2)
{
	return (t2.x - t1.x)*(t2.x-t1.x) + (t2.y-t1.y)*(t2.y-t1.y);
}
inline int dist(const double x1,const double y1,const double x2,const double y2)
{
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}
const bool veh_t[5]{0,1,1,0,0};

const int views[5][3]={{8,8,6},{15,12,9},{13,10,8},{10,10,8},{10,10,8}};
//const int views[5][3]={{15,15,12},{30,24,18},{25,20,15},{20,20,16},{20,20,16}};
const int views_precise[5][3]={{60,60,48},{120,96,72},{100,80,60},{80,80,64},{80,80,64}};
struct tile
{
	int env[2];
	unordered_set <int> ground;//groups
	unordered_set <int> air;//groups
};
static tile tiles[TILES_COUNT][TILES_COUNT];

struct my_unit_str
{
	uint8_t *value;
	VehicleType type;
	int last_move=0;
	int hp;
	bool is_selected=false;
	tile_coord at;
	vector <int> groups;
	double x,y; //need for move detection
};
struct opp_unit_str
{
	uint8_t *value;
	VehicleType type;
	uint8_t hp;
	tile_coord at;
};

struct fac_str
{
	unordered_set <int> cap_grps;
	long long id;
	tile_coord at;
	FacilityType f_type;
	int unassigned=0;
	int progress;
	VehicleType v_type;
	enm_fac_status status=no_status;
	double points;
	double left,top;
	long long owner;
};
inline int distFac(const tile_coord& t1,const fac_str& t2)//const tile_coord t2)
{
	return (t2.at.x - t1.x)*(t2.at.x-t1.x) + (t2.at.y-t1.y)*(t2.at.y-t1.y);
}
static vector <fac_str> facs;

struct general_str
{
	int dead_cnt=0;
	int points=0;
	long long myid;
	Player opp;
	double opp_nuke_atX,opp_nuke_atY;
	tile_coord base;
	int ticks=0;
	general_str()
	{
		base.x=256/TILE_SIZE;base.y=256/TILE_SIZE;
	}
	void calcBase()
	{
		if(facs.empty())
		{
			base.x=256/TILE_SIZE;
			base.y=256/TILE_SIZE;
			return;
		}
		tile_coord p;
		p.x=0;
		p.y=0;
		uint16_t tempX=0,tempY=0;
		size_t size=facs.size()/2;
		sort(begin(facs),
			 end(facs),
			 [p](const fac_str &lhs,const  fac_str &rhs){ return distFac(p,lhs) < distFac(p, rhs); });
		for(size_t i=0;i<size;++i)
		{
			tempX+=facs[i].at.x;
			tempY+=facs[i].at.y;
		}
		base.x=tempX/size+64/TILE_SIZE;
		base.y=tempY/size+64/TILE_SIZE;
	}
	void cntDead(const Player& me)
	{
		dead_cnt=me.getScore()-points;
		//points=me.getScore();
	}
}static gen;
static unordered_map <long long,my_unit_str> my_units;
static unordered_map <long long,opp_unit_str> opp_units;
struct pf_str
{
	int last_update=0;
	//const int pows[16]={2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};
	//const int pows[32]={65536,32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	const  long long pows[64]={4294967294,2147483648,1073741824,536870912,268435456,134217728,67108864,33554432,16777216,
							   8388608,4194304,2097152,1048576,524288,262144,131072,65536,32768,16384,8192,4096,2048,1024,512,256,128,64,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,
							   17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
	//	long long heli_tank[64];
	//	long long heli_arrv[64];
	const long long small[8]={8192,4096,2048,1024,512,256,128,64};
	long long fighter_heli[64];
	long long ifv_heli[64];
	const double h_t=1.2,h_a=1.8,h_f=-1.6,h_i=-0.5,
	f_h=1.6,f_i=-0.3,
	i_h=1.2,i_f=1.1,i_a=1.4,i_t=-1.4,
	t_a=1.5,t_i=1.4,t_h=-1.2,
	a_t=-1.5,a_h=-1.8,a_i=-1.4;

	unordered_map <int,long long> pfs[PF_SIZE][PF_SIZE];
	//	long long getMaxPot(int x,int y,VehicleType v_type)
	//	{
	//		static long long max_pot,temp_pot;
	//		max_pot=getPot(x/TILE_COEF,y/TILE_COEF,v_type);
	//		for(int i=-3;i<4;++i)
	//		{
	//			for(int j=-3;j<4;++j)
	//			{
	//				if((x/TILE_COEF+i<0)||(x/TILE_COEF+i>PF_SIZE_0)||(y/TILE_COEF+j<0)||(y/TILE_COEF+j>PF_SIZE_0))continue;
	//				temp_pot=getPot(x/TILE_COEF+i,y/TILE_COEF+j,v_type);
	//				if(max_pot<temp_pot)
	//				{
	//					max_pot=temp_pot;
	//					tempX=median.x+i*TILE_COEF;
	//					tempY=median.y+j*TILE_COEF;
	//				}
	//			}
	//		}
	//		return max_pot;
	//	}
	long long getPot(int x, int y,VehicleType v_type=VehicleType::_UNKNOWN_)
	{
		static long long pot;
		pot=0;
		switch(v_type)
		{
		case VehicleType::HELICOPTER:
			//pot+=pfs[x][y][ALLY];
			pot+=pfs[x][y][HELI]*0.1;
			pot+=pfs[x][y][TANK]*h_t;
			pot+=pfs[x][y][ARRV]*h_a;
			pot+=pfs[x][y][FIGHTER]*h_f;
			pot+=pfs[x][y][IFV]*h_i;
			break;
		case VehicleType::FIGHTER:
			//pot+=pfs[x][y][ALLY];
			pot+=pfs[x][y][FIGHTER]*0.1;
			pot+=pfs[x][y][HELI]*f_h;
			pot+=pfs[x][y][IFV]*f_i;
			break;
		case VehicleType::TANK:
			pot+=pfs[x][y][ALLY];
			pot+=pfs[x][y][TANK]*0.1;
			pot+=pfs[x][y][ARRV]*t_a;
			pot+=pfs[x][y][IFV]*t_i;
			pot+=pfs[x][y][HELI]*t_h;
			break;
		case VehicleType::IFV:
			pot+=pfs[x][y][ALLY];
			pot+=pfs[x][y][IFV]*0.1;
			pot+=pfs[x][y][ARRV]*i_a;
			pot+=pfs[x][y][TANK]*i_t;
			pot+=pfs[x][y][HELI]*i_h;
			pot+=pfs[x][y][FIGHTER]*i_f;
			break;
		case VehicleType::ARRV:
			pot+=pfs[x][y][TANK]*a_t;
			pot+=pfs[x][y][IFV]*a_i;
			pot+=pfs[x][y][HELI]*a_h;
			break;
		default:
			pot+=pfs[x][y][ALLY];
			break;
		}
		return pot;
	}
	long long getPot2(int x, int y,VehicleType v_type)
	{
		static long long pot;
		pot=0;
		switch(v_type)
		{
		case VehicleType::HELICOPTER:
			pot+=pfs[x][y][HELI];
			pot+=pfs[x][y][TANK]*h_t;
			pot+=pfs[x][y][ARRV]*h_a;
			pot+=pfs[x][y][FIGHTER]*h_f;
			pot+=pfs[x][y][IFV]*h_i;
			break;
		case VehicleType::FIGHTER:
			pot+=pfs[x][y][FIGHTER];
			pot+=pfs[x][y][HELI]*f_h;
			pot+=pfs[x][y][IFV]*f_i;
			//cout<<x<<" "<<y<<": "<<pot<<endl;
			break;
		case VehicleType::TANK:
			pot+=pfs[x][y][TANK];
			pot+=pfs[x][y][ARRV]*t_a;
			pot+=pfs[x][y][IFV]*t_i;
			pot+=pfs[x][y][HELI]*t_h;
			break;
		case VehicleType::IFV:
			pot+=pfs[x][y][IFV];
			pot+=pfs[x][y][ARRV]*i_a;
			pot+=pfs[x][y][TANK]*i_t;
			pot+=pfs[x][y][HELI]*i_h;
			pot+=pfs[x][y][FIGHTER]*i_f;
			break;
		case VehicleType::ARRV:
			pot+=pfs[x][y][TANK]*a_t;
			pot+=pfs[x][y][IFV]*a_i;
			pot+=pfs[x][y][HELI]*a_h;
			break;
		}
		return pot;
	}
	void calcFields3()
	{
		for(int i=0;i<PF_SIZE;++i)
		{
			for (int j=0;j<PF_SIZE;++j)
			{
				pfs[i][j][TANK]=0;
				pfs[i][j][HELI]=0;
				pfs[i][j][FIGHTER]=0;
				pfs[i][j][ARRV]=0;
				pfs[i][j][IFV]=0;
				pfs[i][j][ALLY]=0;
			}
		}
		for(auto &it:my_units)
		{
			for(int i=0;i<PF_SIZE;++i)
			{
				for (int j=0;j<PF_SIZE;++j)
				{
					if(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF))<4)continue;
					pfs[i][j][ALLY]+=64-(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)));
				}
			}

		}
		for(auto &it:opp_units)
		{
			if(it.second.type==VehicleType::HELICOPTER)
			{
				for(int i=-8;i<9;++i)
				{
					for (int j=-8;j<9;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][HELI]+=small[abs(i)+abs(j)];
					}
				}
			}
			else if(it.second.type==VehicleType::TANK)
			{
				for(int i=-6;i<7;++i)
				{
					for (int j=-6;j<7;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][TANK]+=small[abs(i)+abs(j)];
					}
				}
			}
			else if(it.second.type==VehicleType::FIGHTER)
			{
				for(int i=-8;i<9;++i)
				{
					for (int j=-8;j<9;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][FIGHTER]+=small[abs(i)+abs(j)];
					}
				}
			}
			else if(it.second.type==VehicleType::ARRV)
			{
				for(int i=-6;i<7;++i)
				{
					for (int j=-6;j<7;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][ARRV]+=small[abs(i)+abs(j)];
					}
				}
			}
			else if(it.second.type==VehicleType::IFV)
			{
				for(int i=-6;i<7;++i)
				{
					for (int j=-6;j<7;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][IFV]+=small[abs(i)+abs(j)];
					}
				}
			}
		}
	}
	void calcFields2()
	{
		for(int i=0;i<PF_SIZE;++i)
		{
			for (int j=0;j<PF_SIZE;++j)
			{
				pfs[i][j][TANK]=0;
				pfs[i][j][HELI]=0;
				pfs[i][j][FIGHTER]=0;
				pfs[i][j][ARRV]=0;
				pfs[i][j][IFV]=0;
			}
		}
		for(auto &it:opp_units)
		{
			if(it.second.type==VehicleType::HELICOPTER)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][HELI]+=pows[static_cast<long long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::TANK)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][TANK]+=pows[static_cast<long long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::FIGHTER)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][FIGHTER]+=pows[static_cast<long long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::ARRV)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][ARRV]+=pows[static_cast<long long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::IFV)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][IFV]+=pows[static_cast<long long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
		}
	}
	void calcPosFields()
	{
		for(int i=0;i<PF_SIZE;++i)
		{
			for (int j=0;j<PF_SIZE;++j)
			{
				pfs[i][j][TANK]=0;
				pfs[i][j][HELI]=0;
				pfs[i][j][FIGHTER]=0;
				pfs[i][j][ARRV]=0;
				pfs[i][j][IFV]=0;
			}
		}
		for(auto &it:opp_units)
		{
			if(it.second.type==VehicleType::HELICOPTER)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][HELI]+=128-(abs(i-round(it.second.at.x/2))+abs(j-round(it.second.at.y/2)));
					}
				}
			}
			else if(it.second.type==VehicleType::TANK)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][TANK]+=128-(abs(i-round(it.second.at.x/2))+abs(j-round(it.second.at.y/2)));
					}
				}
			}
			else if(it.second.type==VehicleType::FIGHTER)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][FIGHTER]+=128-(abs(i-round(it.second.at.x/2))+abs(j-round(it.second.at.y/2)));
					}
				}
			}
			else if(it.second.type==VehicleType::ARRV)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][ARRV]+=128-(abs(i-round(it.second.at.x/2))+abs(j-round(it.second.at.y/2)));
					}
				}
			}
			else if(it.second.type==VehicleType::IFV)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][IFV]+=128-(abs(i-round(it.second.at.x/2))+abs(j-round(it.second.at.y/2)));
					}
				}
			}
		}
	}
	void calcFields()
	{
		for(int i=0;i<PF_SIZE;++i)
		{
			for (int j=0;j<PF_SIZE;++j)
			{
				pfs[i][j][TANK]=0;
				pfs[i][j][HELI]=0;
				pfs[i][j][FIGHTER]=0;
				pfs[i][j][ARRV]=0;
				pfs[i][j][IFV]=0;
			}
		}
		for(auto &it:opp_units)
		{
			if(it.second.type==VehicleType::HELICOPTER)
			{
				for(int i=-5;i<6;++i)
				{
					for(int j=-5;j<6;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][ARRV]=0;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][TANK]=0;
					}
				}
				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][HELI]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
						pfs[i][j][FIGHTER]+=fighter_heli[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
						pfs[i][j][IFV]+=ifv_heli[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::TANK)
			{
				for(int i=-5;i<6;++i)
				{
					for(int j=-5;j<6;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][ARRV]=0;
					}
				}
				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][HELI]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::FIGHTER)
			{
				for(int i=-5;i<6;++i)
				{
					for(int j=-5;j<6;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][HELI]=0;
					}
				}

				//	for(int i=0;i<PF_SIZE;++i)
				//{
				//	for (int j=0;j<PF_SIZE;++j)
				//	{

				//pfs[i][j][FIGHTER]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
				//	}
				//}
			}
			else if(it.second.type==VehicleType::ARRV)
			{

				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][TANK]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
						pfs[i][j][HELI]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
			else if(it.second.type==VehicleType::IFV)
			{
				for(int i=-5;i<6;++i)
				{
					for(int j=-5;j<6;++j)
					{
						if((it.second.at.x/TILE_COEF+i<0)||(it.second.at.x/TILE_COEF+i>PF_SIZE_0)||(it.second.at.y/TILE_COEF+j<0)||(it.second.at.y/TILE_COEF+j>PF_SIZE_0))continue;
						pfs[it.second.at.x/TILE_COEF+i][it.second.at.y/TILE_COEF+j][ARRV]=0;
					}
				}
				for(int i=0;i<PF_SIZE;++i)
				{
					for (int j=0;j<PF_SIZE;++j)
					{
						pfs[i][j][TANK]+=pows[static_cast<unsigned long>(abs(i-round(it.second.at.x/TILE_COEF))+abs(j-round(it.second.at.y/TILE_COEF)))];
					}
				}
			}
		}
	}
}static pf_map;
static enm_action wait_move=no_act;
static int selected=NO;
struct group_str
{
	static int ROUTE_RATE;
	static int new_offset_x;
	bool air=false;
	//bool healing=false;
	bool nuke=false;
	bool idle=true;
	VehicleType v_type;
	unordered_set <long long> ids;
	int last_scale=100;
	int last_rotate=300;
	int nuke_start_tick=0;
	enm_grp_move_status movement;
	enm_grp_task task=no;
	int grp_id;
	int dirX,dirY;
	enm_vehs hunt_target=_UNKNOWN_;
	int following=-1;
	int follow_dist=0;
	tile_coord base_offset;
	tile_coord target;
	tile_coord median;

	int under_hp=0;// amount % of units lower then X hp
	int ave_hp=100;
	enm_headings heading;
	group_str(int grp_id,enm_grp_move_status movement)
	{
		base_offset.x=0;
		base_offset.y=0;
		this->grp_id=grp_id;this->movement=movement;
	}
	bool calcMedian()
	{
		if(ids.empty())return 0;
		int n=0;
		median.x=0;
		median.y=0;
		for(auto &i:ids)
		{
			n++;
			median.x+=my_units[i].at.x;
			median.y+=my_units[i].at.y;
		}
		median.x=median.x/n;
		median.y=median.y/n;
		return 1;
	}
	int route ();
	bool tileEmpty(int offsetX,int offsetY)
	{
#ifdef dbg
		cout<<"Grp:"<<grp_id<<" X:"<<dirX<<" Y:"<<dirY<<" offX:"<<offsetX<<" offY:"<<offsetY;
#endif
		if(!air)
		{
			for(auto &i:ids)
			{
				if((my_units[i].at.x+offsetX<0)||(my_units[i].at.x+offsetX>TILES_COUNT_0)||(my_units[i].at.y+offsetY<0)||(my_units[i].at.y+offsetY>TILES_COUNT_0))return 0;
				for(auto &it:tiles[my_units[i].at.x+offsetX][my_units[i].at.y+offsetY].ground)
				{

					if(it!=grp_id)
					{
#ifdef dbg
						cout<<"\033[1;33m Crosses grd \033[1;0m"<<it<<" at x:"<<my_units[i].at.x+offsetX<<" y:"<<my_units[i].at.y+offsetY<<endl;
#endif
						return 0;
					}
				}
			}
		}
		else
			for(auto &i:ids)
			{

				if((my_units[i].at.x+offsetX<0)||(my_units[i].at.x+offsetX>TILES_COUNT_0)||(my_units[i].at.y+offsetY<0)||(my_units[i].at.y+offsetY>TILES_COUNT_0))return 0;
				for(auto &it:tiles[my_units[i].at.x+offsetX][my_units[i].at.y+offsetY].air)
				{
					if(it!=grp_id)
					{
#ifdef dbg
						cout<<"\033[1;33m Crosses air \033[1;0m"<<it<<" at x:"<<my_units[i].at.x+offsetX<<" y:"<<my_units[i].at.y+offsetY<<endl;
#endif
						return 0;
					}
				}
			}
#ifdef dbg
		cout<<"\033[1;32m Empty\033[1;0m"<<endl;
#endif

		return 1;
	}
	void downscale(model::Move& move)
	{
		calcMedian();
		move.setAction(ActionType::SCALE);
		move.setX(median.x*TILE_SIZE);
		move.setY(median.y*TILE_SIZE);
		move.setFactor(0.1);
	}
	void rotate(model::Move& move)
	{
		static double angle=1.4;
		calcMedian();
		move.setAction(ActionType::ROTATE);
		move.setX(median.x*TILE_SIZE);
		move.setY(median.y*TILE_SIZE);
		move.setAngle(angle);
		angle=-angle;
	}
	static void setCapTargets();
	void calcAveHp()
	{
		static int summ,n;
		summ=0;
		n=0;
		for(auto &i:ids)
		{
			summ+=my_units.at(i).hp;
			++n;
		}
		ave_hp=summ/n;
	}
	void calcUnderHp(int edge)
	{
		static int n;
		n=0;
		for (auto &i:ids)
		{
			if(my_units.at(i).hp<edge)++n;
		}
		//if(n==0){under_hp=0;return;}
		under_hp=nearbyint((100/ids.size())*n);
	}
	bool needHealing(int pct,int edge, int comb_pct, int comb_edge)
	{
		if(((ave_hp<pct)||(under_hp>edge))||
				((ave_hp<comb_pct)&&(under_hp<comb_edge)))return 1;
		return 0;
	}
	void go2base()
	{
		movement=NEW_ROUTE;
		task=move2base;
		target.x=gen.base.x+base_offset.x;
		target.y=gen.base.y+base_offset.y;
	}
	void setFollow(int grp,int dist=0);
	void isIdle()
	{
		for(auto &i:ids)
		{
			if(my_units.at(i).last_move==gen.ticks)
			{
				idle=false;
				return;
			}
		}
		idle=true;
	}
};
static unordered_map <int,group_str>groups;
inline void initFac(const World & world)
{
	vector <Facility>fac_vec;
	fac_vec=world.getFacilities();
	for(auto &i:fac_vec)
	{
		facs.emplace(facs.end());
		facs.back().id=i.getId();
		facs.back().f_type=i.getType();
		facs.back().left=i.getLeft();
		facs.back().top=i.getTop();
		facs.back().at.x=round((i.getLeft()+32)/TILE_SIZE);
		facs.back().at.y=round((i.getTop()+32)/TILE_SIZE);
	}
}
inline void updateFac(const World & world,const model::Player& me)
{
	static vector <Facility>fac_vec;
	fac_vec=world.getFacilities();
	for(auto &i:fac_vec)
	{
		static vector <fac_str>::iterator it;
		for(it=facs.begin();it!=facs.end();++it)
		{
//			for(auto &i:it->cap_grps)
//			{
//				if(groups.at(i).ids.empty())
//				{
//					groups.at(i).task=no;
//					it->cap_grps.erase(i);
//				}
//			}
//			if(it->cap_grps.empty())
//			{
//				it->status=no_status;
//			}
			if (it->id==i.getId())
			{
				if(it->owner!=i.getOwnerPlayerId())
				{
					if(me.getId()==i.getOwnerPlayerId())//become mine
					{
						gen.points+=100;
						it->status=captured;
						for(auto &i:it->cap_grps)
						{
							groups.at(i).task=no;
						}
						it->cap_grps.clear();
					}
					else if(me.getId()!=i.getOwnerPlayerId())//become not mine
					{
						it->status=no_status;
					}
				}
				it->owner=i.getOwnerPlayerId();
				it->v_type=i.getVehicleType();
				it->points=i.getCapturePoints();
				it->progress=i.getProductionProgress();
			}
		}

		//		facs.back().f_type=i.getType();
		//		facs.back().owner=i.getOwnerPlayerId();
		//		facs.back().v_type=i.getVehicleType();
		//		facs.back().points=i.getCapturePoints();
		//		facs.back().progress=i.getProductionProgress();
	}
}

inline bool isIdle(my_unit_str &unit)
{
	return unit.last_move!=gen.ticks;
}
inline bool isIdle(unordered_map <long long,my_unit_str> &units,VehicleType type=VehicleType::_UNKNOWN_)
{
	for(auto &i :units )
	{
		if((i.second.type==type)||(type==VehicleType::_UNKNOWN_))
		{
			if(i.second.last_move==gen.ticks)return 0;
		}
	}
	return 1;
}
static vector<VehicleUpdate> updates_arr;
inline void updateVeh(const World & world)
{
	updates_arr=world.getVehicleUpdates();
	static tile dummy;
	//chrono::steady_clock::time_point t1,t2;
	//clock_t t = clock();


	//t1= chrono::steady_clock::now();
	for(int i=0;i<TILES_COUNT;++i)
	{
		for(int j=0;j<TILES_COUNT;++j)
		{
			tiles[i][j].air.clear();
			tiles[i][j].ground.clear();
		}
	}

	//t2= chrono::steady_clock::now();
	//cout << "cpu "<<clock()-t<<", time " << chrono::duration_cast<chrono::microseconds> (t2 - t1).count() <<endl;

	for(auto &n:updates_arr)
	{
		long long id=n.getId();

		if(my_units.end() != my_units.find(id))
		{
			if(0==n.getDurability())
			{
				for(auto &i:my_units.at(id).groups)
				{
					if(-1==i)continue;
					groups.at(i).ids.erase(id);
				}
				my_units.erase(id);
				continue;
			}

			if((!areEqual(my_units[id].x,n.getX()))||(!areEqual(my_units[id].y,n.getY())))
			{
				my_units[id].last_move=gen.ticks;
			}
			my_units[id].x=n.getX();
			my_units[id].y=n.getY();
			my_units[id].hp=n.getDurability();
			my_units[id].groups=n.getGroups();
			my_units[id].is_selected=n.isSelected();
		}
		else //opp units
		{
			if(0==n.getDurability())
			{
				opp_units.erase(id);
				continue;
			}
			opp_units[id].at.x=round(n.getX()/TILE_SIZE);
			if(opp_units[id].at.x>TILES_COUNT_0)opp_units[id].at.x=TILES_COUNT_0;
			opp_units[id].at.y=round(n.getY()/TILE_SIZE);
			if(opp_units[id].at.y>TILES_COUNT_0)opp_units[id].at.y=TILES_COUNT_0;
			opp_units[id].hp=n.getDurability();
		}
	}
	//t1= chrono::steady_clock::now();
	for(auto &n:my_units)
	{
		n.second.at.x=round(n.second.x/TILE_SIZE);
		if(n.second.at.x>TILES_COUNT_0)n.second.at.x=TILES_COUNT_0;
		n.second.at.y=round(n.second.y/TILE_SIZE);
		if(n.second.at.y>TILES_COUNT_0)n.second.at.y=TILES_COUNT_0;
		//if((n.second.at.x<0)||(n.second.at.x>TILES_COUNT_0)||(n.second.at.y<0)||(n.second.at.y>TILES_COUNT_0))continue;
		if((n.second.type==VehicleType::HELICOPTER)||(n.second.type==VehicleType::FIGHTER))
		{
			if(n.second.groups.empty())
			{
				tiles[n.second.at.x][n.second.at.y].air.emplace(-1);
			}
			else for( auto &grp:n.second.groups)
			{
				//cout<<n.first<<" "<<n.second.at.x<<" "<<n.second.at.y<<endl;
				tiles[n.second.at.x][n.second.at.y].air.emplace(grp);
			}
		}
		else
		{
			if(n.second.groups.empty())
			{
				tiles[n.second.at.x][n.second.at.y].ground.emplace(-1);
			}
			else for( auto &grp:n.second.groups)
			{
				tiles[n.second.at.x][n.second.at.y].ground.emplace(grp);
			}
		}
		for( auto &grp:n.second.groups)
		{
			//if(grp==-1)continue;
			groups.at(grp).ids.emplace(n.first);
		}
	}
	//t2= chrono::steady_clock::now();
	//cout << "cpu "<<clock()-t<<", time " << chrono::duration_cast<chrono::microseconds> (t2 - t1).count() <<endl;

}

struct nuke_str
{
	int edge=500;
	tile_coord at;
	int hit_value;
	int hit_value_prev=0;
	int grp_id;
	long long aimer_id;
	int nuke_field[TILES_COUNT][TILES_COUNT];
	int calc_rate=4;
	bool idle=false;
	bool calcNuke()
	{
		static unordered_map <long long,my_unit_str>::iterator it;
		hit_value=0;
		fill_n(&nuke_field[0][0],TILES_SQUARED,0);
		for(auto &it:opp_units)
		{
			for(int i=-6;i<7;++i)
			{
				for(int j=-6;j<7;++j)
				{
					if((it.second.at.x+i<0)||(it.second.at.x+i>TILES_COUNT_0)||(it.second.at.y+j<0)||(it.second.at.y+j)>TILES_COUNT_0)continue;
					nuke_field[it.second.at.x+i][it.second.at.y+j]+=13-abs(i)-abs(j);
				}
			}
		}
		for(it=my_units.begin();it!=my_units.end();++it)
		{
			for(int i=-6;i<7;++i)
			{
				for(int j=-6;j<7;++j)
				{
					if((it->second.at.x+i<0)||(it->second.at.x+i>TILES_COUNT_0)||(it->second.at.y+j<0)||(it->second.at.y+j)>TILES_COUNT_0)continue;
					nuke_field[it->second.at.x+i][it->second.at.y+j]-=13-abs(i)-abs(j);
				}
			}
		}
		for(it=my_units.begin();it!=my_units.end();++it)
		{
			if(it->second.groups.empty())continue;
			if(it->second.hp<31)continue;
			for(int i=-views[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]];
				i<=views[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]];++i)
			{
				for(int j=-views[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]];
					j<=views[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]];++j)
				{
					if(abs(i)+abs(j)>views[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]])continue;
					if((it->second.at.x+i<0)||(it->second.at.x+i>TILES_COUNT_0)||(it->second.at.y+j<0)||(it->second.at.y+j)>TILES_COUNT_0)continue;
					if(hit_value<nuke_field[it->second.at.x+i][it->second.at.y+j])
					{
						int D=dist(it->second.x,it->second.y,(it->second.at.x+i)*TILE_SIZE,(it->second.at.y+j)*TILE_SIZE);
						if(D>views_precise[static_cast <int>(it->second.type)][tiles[it->second.at.x][it->second.at.y].env[veh_t[static_cast <int>(it->second.type)]]])
						{
							//cout<<"not valid dist:"<<D<<endl;
							continue;
						}
						hit_value=nuke_field[it->second.at.x+i][it->second.at.y+j];
						at.x=it->second.at.x+i;
						at.y=it->second.at.y+j;
						aimer_id=it->first;
					}
				}
			}
		}
#ifdef dbg
		cout<<"Hit value: "<<hit_value<<" "<<hit_value_prev<<endl;
#endif
		edge=700-gen.dead_cnt;//*max_element(nuke_field[0],nuke_field[0]+TILES_SQUARED);

		if(hit_value>edge)
		{
			calc_rate=1;
			if(hit_value<hit_value_prev)
			{
				for (auto &it:my_units.at(aimer_id).groups)
				{
					groups.at(it).nuke=true;
					grp_id=it;
				}
				calc_rate=4;
				hit_value_prev=0;
				cout<<"tick"<<gen.ticks<<" nuke x:"<<at.x<<" y:"<<at.y<<" value:"<<hit_value<<" edge:"<<edge<<"units dead: "<<gen.dead_cnt<<endl;
				cout<<my_units[aimer_id].at.x<<" y:"<<my_units[aimer_id].at.y<<" dbl x:"<<my_units[aimer_id].x<<" y:"<<my_units[aimer_id].y<<endl;
				cout<<"AIM dist : "<<dist(my_units[aimer_id].x,my_units[aimer_id].y,at.x*TILE_SIZE,at.y*TILE_SIZE)<<" TYPE: "<<static_cast<int>(my_units[aimer_id].type)<<endl;
				return 1;
			}
		}
		hit_value_prev=hit_value;
		return 0;
	}
}static nuke;


struct mission_str
{
	uint8_t stage=0;
	enm_mission_type type;
	mission_str(enm_mission_type type):type(type){}
	int m_route(const Player &me, model::Move& move);
	virtual int m_startFormation(model::Move& move,const model::World& world){return 1;}
	virtual ~mission_str(){}
	inline int m_setProd(const model::Player& me,model::Move& move)
	{
		for(auto &i:facs)
		{
			if(i.f_type==FacilityType::CONTROL_CENTER)continue;
			if(i.owner==me.getId()&&i.status!=producing)
			{
				i.status=producing;
				move.setFacilityId(i.id);
				move.setVehicleType(VehicleType::TANK);
				move.setAction(ActionType::SETUP_VEHICLE_PRODUCTION);
				return 0;
			}
		}
		return -1;
	}
};
struct start_str:mission_str
{
	long long tank=-1,ifv=-1,arrv=-1,fighter=-1,heli=-1;
	using mission_str::mission_str;
	int m_startFormation(model::Move& move,const model::World& world);
	void init(const model::World& world);

};
static multimap <enm_priority,mission_str*> mission_q;

#ifdef nc
inline void nc_inters()
{
	static int ch,offsetY,offsetX;
	ch=getch();
	switch(ch)
	{
	case 98://b
		ch=0;
		break;
	case 32://space
		while(getch()!=32){usleep(5);}
		break;
		//	case 97:
		//		if(offsetX<11)offsetX=0;
		//		else offsetX-=10;
		//		break;
		//	case 100:
		//		if(offsetX<170)
		//			offsetX+=10;
		//		break;
	case 119:
		if(offsetY<11)offsetY=0;
		else offsetY-=10;
		break;
	case 115:
		if(offsetY<170)
			offsetY+=10;
		break;
	}
	//}
	move(0,0);
	static int i,j;
	printw(" ");
	for(int n =0;n<256;++n)
	{
		if(n%10==0)printw("%i",n);
		else if(n%9!=0)	printw(" ");
	}
	for(i=0+offsetY;i<80+offsetY;++i)
	{
		attron(COLOR_PAIR(4));printw("%i",i);

		for(j =0;j<250;++j)
		{
			if(!tiles[j][i].air.empty()){attron(COLOR_PAIR(2));printw("%i",tiles[j][i].air.size());}
			else if(!tiles[j][i].ground.empty()){attron(COLOR_PAIR(3));printw("%i",tiles[j][i].ground.size());}
			else {attron(COLOR_PAIR(1));printw(" ");}
		}

		printw("\n");
	}
}
inline void nc_pfs()
{
	static int ch,offsetY,offsetX;
	ch=getch();
	switch(ch)
	{
	case 98://b
		ch=0;
		break;
	case 32://space
		while(getch()!=32){usleep(5);}
		break;
	case 97:
		if(offsetX<11)offsetX=0;
		else offsetX-=10;
		break;
	case 100:
		if(offsetX<170)
			offsetX+=10;
		break;
	case 119:
		if(offsetY<11)offsetY=0;
		else offsetY-=10;
		break;
	case 115:
		if(offsetY<170)
			offsetY+=10;
		break;
	}
	//}
	move(0,0);
	static int i,j;
	printw(" ");
	for(int n =0;n<64;++n)
	{
		if(n%10==0)printw("%i",n);
		else if(n%9!=0)	printw(" ");
	}
	printw("\n");
	for(i=0;i<64;++i)
	{
		//	attron(COLOR_PAIR(4));
		//	printw("%i",i);

		for(j=0;j<64;++j)
		{
			attron(COLOR_PAIR(2));printw("%i ",pf_map.pfs[i][j][HELI]);

			//else if(!tiles[j][i].ground.empty()){attron(COLOR_PAIR(3));printw("%i",tiles[j][i].ground.size());}
			//else {attron(COLOR_PAIR(1));printw(" ");}
			//			if (tiles[j][i].count>0)attron(COLOR_PAIR(1));
			//else attron(COLOR_PAIR(2));
			//	printw("%i",tiles[j][i].count);
		}

		printw("\n");
	}

}
inline void nc_nuke()
{
	static int ch,offsetY,offsetX;
	ch=getch();
	switch(ch)
	{
	case 98://b
		ch=0;
		break;
	case 32://space
		while(getch()!=32){usleep(5);}
		break;
		//	case 97:
		//		if(offsetX<11)offsetX=0;
		//		else offsetX-=10;
		//		break;
		//	case 100:
		//		if(offsetX<170)
		//			offsetX+=10;
		//		break;
	case 119:
		if(offsetY<11)offsetY=0;
		else offsetY-=10;
		break;
	case 115:
		if(offsetY<170)
			offsetY+=10;
		break;
	}
	//}
	move(0,0);
	static int i,j;
	printw(" ");
	for(int n =0;n<256;++n)
	{
		if(n%10==0)printw("%i",n);
		else if(n%9!=0)	printw(" ");
	}
	for(i=0+offsetY;i<80+offsetY;++i)
	{
		attron(COLOR_PAIR(4));printw("%i",i);

		for(j =0;j<220;++j)
		{
			//if(tiles[j][i].opp_air){attron(COLOR_PAIR(2));printw("1");}
			if(nuke.nuke_field[j][i]!=0)
			{attron(COLOR_PAIR(2));printw("%i ",nuke.nuke_field[j][i]);}
			//	else {attron(COLOR_PAIR(1));printw(" ");}

		}

		printw("\n");
	}
}
#endif
static int turns_arr[12]={59,59,59,59,59,59,59,59,59,59,59,59};
static int turns;
inline int gotTurns()
{
	static int n;
	n=0;
	for(int i=0;i<12;++i)
	{
		if (turns_arr[i]>59)++n;
	}
	return n;
}
inline void reduceTurns()
{
	for(int i=0;i<12;++i)
	{
		if (turns_arr[i]>59)
		{
			turns_arr[i]=0;
			return;
		}
	}
}
inline void updateTurns()
{
	for(int i=0;i<12;++i)
	{
		++turns_arr[i];
	}
}


class MyStrategy : public Strategy {
public:
	MyStrategy();

	void move(const model::Player& me, const model::World& world, const model::Game& game, model::Move& move) override;
};

#endif

