#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

class OneLoneCoder_FormulaOLC : public olcConsoleGameEngine 
{

public:
	OneLoneCoder_FormulaOLC() 
	{
		m_sAppName = L"Formula OLC";
	}

private:

	float fCarPos = 0.0f;
	float fDistance = 0.0f;
	float fSpeed = 0.0f;

	float fCurvature = 0.0f;
	float fTrackCurvature = 0.0f;
	float fPlayerCurvature = 0.0f;
	float fTrackDistance = 0.0f;

	float fCurrentLapTime;

	vector<pair<float, float>> vecTrack; //curvature, distance
	list<float> listLapTimes;

protected:
	//Called by oclConsoleGameEngine
	virtual bool OnUserCreate() 
	{

		//Start to create the "track"
		vecTrack.push_back(make_pair(0.0f, 10.0f)); //Short section for start/finish
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f));
		vecTrack.push_back(make_pair(0.0f, 400.0f));
		vecTrack.push_back(make_pair(-1.0f, 100.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(-1.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f));

		for (auto t : vecTrack)
			fTrackDistance += t.second;

		listLapTimes = { 0,0,0,0,0 };
		fCurrentLapTime = 0.0f;
		

		return true;
	}

	//Called by oclConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime) {

		if (m_keys[VK_UP].bHeld) 
			fSpeed += 2.0f * fElapsedTime;
		else
			fSpeed -= 1.0f * fElapsedTime;

		int nCarDirection = 0;

		if (m_keys[VK_LEFT].bHeld)
		{
			fPlayerCurvature -= 0.7f * fElapsedTime;
			nCarDirection = -1;
		}
		if (m_keys[VK_RIGHT].bHeld)
		{
			fPlayerCurvature += 0.7f * fElapsedTime;
			nCarDirection = 1;
		}

		//If player has gone off the track slow down the car
		if (fabs(fPlayerCurvature - fTrackCurvature) >= 0.8f)
			fSpeed -= 5.0f * fElapsedTime;

		//Clamp Speed
		if (fSpeed < 0.0f) fSpeed = 0.0f;
		if (fSpeed > 1.0f) fSpeed = 1.0f;

		//Move car along track according to car speed
		fDistance += (140.0f * fSpeed) * fElapsedTime;
		
		//Get point on track
		float fOffset = 0.0f;
		int nTrackSection = 0;

		fCurrentLapTime += fElapsedTime;
		if (fDistance >= fTrackDistance)
		{
			fDistance -= fTrackDistance;
			listLapTimes.push_front(fCurrentLapTime);
			listLapTimes.pop_back();
			fCurrentLapTime = 0.0f;
		}
			

		//Find the position on track (could be optimized better)
		while (nTrackSection < vecTrack.size() && fOffset <= fDistance)
		{
			fOffset += vecTrack[nTrackSection].second;
			nTrackSection++;
		}

		float fTargetCurvature = vecTrack[nTrackSection - 1].first;

		float fTrackCurveDiff = (fTargetCurvature - fCurvature) * fElapsedTime * fSpeed;
		fCurvature += fTrackCurveDiff;

		fTrackCurvature += (fCurvature)* fElapsedTime * fSpeed;

		// Draw the sky, Light Blue and Dark Blue
		for (int y = 0; y < ScreenHeight() / 2; y++)
			for (int x = 0; x < ScreenWidth(); x++)
				Draw(x, y, y < ScreenHeight() / 4 ? PIXEL_HALF : PIXEL_SOLID, FG_DARK_BLUE);

		// Draw Scenery - our hills are a rectified sine wave, where the phase is adjusted by the
		// accumulated track curvature
		for (int x = 0; x < ScreenWidth(); x++)
		{
			int nHillHeight = (int)(fabs(sinf(x * 0.01f + fTrackCurvature) * 16.0f));
			for (int y = (ScreenHeight() / 2) - nHillHeight; y < ScreenHeight() / 2; y++)
				Draw(x, y, PIXEL_SOLID, FG_DARK_YELLOW);
		}

		for (int y = 0; y < ScreenHeight() / 2; y++) 
		{
			for (int x = 0; x < ScreenWidth(); x++) 
			{
				//Add in perspective
				float fPerspective = (float)y / (ScreenHeight() / 2.0f);

				float fMiddlePoint = 0.5f  + fCurvature * powf((1.0f - fPerspective), 3);

				float fRoadWidth = 0.1f + fPerspective * 0.8f;
				float fClipWidth = fRoadWidth * 0.15f;

				fRoadWidth *= 0.5f;

				//Go from the middle and subtract the road and the curb of the road and what we have left is the grass
				int nLeftGrass = (fMiddlePoint - fRoadWidth - fClipWidth) * ScreenWidth();
				//Go from the middle and subtract out the road and what we have between that and the nLeftGrass is the curb
				int nLeftClip = (fMiddlePoint - fRoadWidth) * ScreenWidth();
				//Same as above but just reversed for right
				int nRightGrass = (fMiddlePoint + fRoadWidth + fClipWidth) * ScreenWidth();
				int nRightClip = (fMiddlePoint + fRoadWidth) * ScreenWidth();

				//Function to make it so that we have "movement" of the grass, closer in distance further apart closer to screen
				int nGrassColor = sinf(20.0f * powf(1.0f - fPerspective, 3) + fDistance * 0.1f) > 0.0f ? FG_GREEN : FG_DARK_GREEN;
				int nClipColor = sinf(80.0f * powf(1.0f - fPerspective, 2) + fDistance) > 0.0f ? FG_RED : FG_WHITE;

				//Make sure we are in the bottom half of the screen, otherwise we would start from top of screen since y = 0
				int nRow = ScreenHeight() / 2 + y;

				//If we are drawing across the screen from bottom left and we are within the "grass" section drraw the pixels green
				if (x >= 0 && x < nLeftGrass)
					Draw(x, nRow, PIXEL_SOLID, nGrassColor);
				//If we are within the clip section then draw the pixels red
				if (x >= nLeftGrass && x < nLeftClip)
					Draw(x, nRow, PIXEL_SOLID, nClipColor);
				//If we are between the left clip and the right clip then this is the road so we will draw it grey
				if (x >= nLeftClip && x < nRightClip)
					Draw(x, nRow, PIXEL_SOLID, FG_GREY);
				//Opposite of the above for the right clip and the right grass
				if (x >= nRightClip && x < nRightGrass)
					Draw(x, nRow, PIXEL_SOLID, nClipColor);
				if (x >= nRightGrass && x < ScreenWidth())
					Draw(x, nRow, PIXEL_SOLID, nGrassColor);
			}
		}

		//Draw the car. Since the car uses -1 as the left side of the screen and +1 as the right the middle is 0, unlike our screen which is 0.5 so we need to scale
		fCarPos = fPlayerCurvature - fTrackCurvature;
		int nCarPos = ScreenWidth() / 2 + ((int)(ScreenWidth() * fCarPos) / 2.0f) - 7;

		switch (nCarDirection)
		{
		case 0:
			DrawStringAlpha(nCarPos, 80, L"   ||####||   ");
			DrawStringAlpha(nCarPos, 81, L"      ##      ");
			DrawStringAlpha(nCarPos, 82, L"     ####     ");
			DrawStringAlpha(nCarPos, 83, L"     ####     ");
			DrawStringAlpha(nCarPos, 84, L"|||  ####  |||");
			DrawStringAlpha(nCarPos, 85, L"|||########|||");
			DrawStringAlpha(nCarPos, 86, L"|||  ####  |||");
			break;

		case +1:
			DrawStringAlpha(nCarPos, 80, L"      //####//");
			DrawStringAlpha(nCarPos, 81, L"         ##   ");
			DrawStringAlpha(nCarPos, 82, L"       ####   ");
			DrawStringAlpha(nCarPos, 83, L"      ####    ");
			DrawStringAlpha(nCarPos, 84, L"///  ####//// ");
			DrawStringAlpha(nCarPos, 85, L"//#######///O ");
			DrawStringAlpha(nCarPos, 86, L"/// #### //// ");
			break;

		case -1:
			DrawStringAlpha(nCarPos, 80, L"\\\\####\\\\      ");
			DrawStringAlpha(nCarPos, 81, L"   ##         ");
			DrawStringAlpha(nCarPos, 82, L"   ####       ");
			DrawStringAlpha(nCarPos, 83, L"    ####      ");
			DrawStringAlpha(nCarPos, 84, L" \\\\\\\\####  \\\\\\");
			DrawStringAlpha(nCarPos, 85, L" O\\\\\\#######\\\\");
			DrawStringAlpha(nCarPos, 86, L" \\\\\\\\ #### \\\\\\");
			break;

		}

		

		//Draw Stats for screen
		DrawString(0, 0, L"Distance: " + to_wstring(fDistance));
		DrawString(0, 1, L"Target Curvature: " + to_wstring(fCurvature));
		DrawString(0, 2, L"Player Curvature: " + to_wstring(fPlayerCurvature));
		DrawString(0, 3, L"Player Speed: " + to_wstring(fSpeed));
		DrawString(0, 4, L"Track Curvature: " + to_wstring(fTrackCurvature));

		auto disp_time = [](float t)
		{
			int nMinutes = t / 60.0f;
			int nSeconds = t - (nMinutes * 60.0f);
			int nMilliSeconds = (t - (float)nSeconds) * 1000.0f;
			return to_wstring(nMinutes) + L"." + to_wstring(nSeconds) + L":" + to_wstring(nMilliSeconds);
		};

		DrawString(10, 8, disp_time(fCurrentLapTime));

		//Display last 5 lap times
		int j = 10;
		for (auto l : listLapTimes)
		{
			DrawString(10, j, disp_time(l));
			j++;
		}

		return true;
	}

};


int main()
{
	//Use olcConsoleGameEngine derived app
	OneLoneCoder_FormulaOLC game;
	game.ConstructConsole(180, 100, 8, 8);
	game.Start();
	//return 0;
}
