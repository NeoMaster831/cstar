#pragma once
#include "pch.h"

enum PlayState {
	Menu, Edit, Play, Exit,
	SelectEdit, SelectPlay, SelectDrawings, Rank,
	Update, Busy, Unknown, Lobby,
	MatchSetup, SelectMulti, RankingVs, OnlineSelection,
	OptionsOffsetWizard, RankingRagCoop, RankingTeam, BeatmapImport,
	PackageUpdater, Benchmark, Tourney, Charts
};



class PState {

public:
	BYTE pState; 
	int timestamp; // in milliseconds
};