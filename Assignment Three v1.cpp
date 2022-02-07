 // Assignment Three v1.cpp: A program using the TL-Engine
 //Bethany Walker - 20745023

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <sstream>
using namespace tle;

struct sMovementVector
{
	float xVector;
	float zVector;
};

void directionCalculation(IModel* car, float thrust, sMovementVector& momentumVect, float frameTime);

bool sphereToBox(IModel* car, IModel* wall);

bool pointToBox(IModel* car, IModel* checkpoint);

bool sphereToSphere(IModel* car, IModel* checkpoint);

bool tankSphereToSphere(IModel* car, IModel* tank);

bool aiDummyCollision(IModel* aICar, IModel* dummy);

bool sideBorderCollision(IModel* car, IModel* sideBarrier);

bool topBorderCollision(IModel* car, IModel* topBarrier);

bool racePosition(IModel* car, IModel* rival, IModel* dummy);

bool playerDummyCollision(IModel* car, IModel* dummy);

 //Controls
const EKeyCode kForward = Key_W;                     
const EKeyCode back = Key_S;
const EKeyCode clockwise = Key_D;
const EKeyCode anticlockwise = Key_A;

const EKeyCode startKey = Key_Space;
const EKeyCode quitKey = Key_Escape;

const EKeyCode cameraUp = Key_Up;
const EKeyCode cameraDown = Key_Down;
const EKeyCode cameraRight = Key_Right;
const EKeyCode cameraLeft = Key_Left;
const EKeyCode cameraReset = Key_1;
const EKeyCode fpCamera = Key_2;

const EKeyCode boost = Key_Space;
const EKeyCode restart = Key_R;
const EKeyCode otherCamera = Key_C;

//Numbers of assests 
const int kNumCheckpoint = 4;
const int kNumIsle = 16;
const int kNumWalls = 72;
const int kNumSideBorders = 7;
const int kNumTopBorders = 3;
const int kNumDummy = 4;
const int kNumAIDummy = 11;
const int kNumTanks = 8;
const int kNumSecondTanks = 5;
const int kNumCrosses = 4;
const float carRadius = 4.0f;
const float aiCarRadius = 2.0f;
const float aiDummyRadius = 40.0f;

//Establishing the damage counter
int hpCounter = 100;
int midHealth = 50;
int lowHealth = 10;
int damage = 1;


//Text Positions
int fontPos = 75;
int stateFontPos = 35;
int instructionPos = 40;

int textPositionX = 50;
int speedY = 45;
int hpY = 85;
int lapY = 125;
int positionY = 160;

int lapTimeX = 50;
int playerLapTimeY = 175;
int rivalLapTimeY = 200;

int winnerX = 700;
int winnerY = 50;

enum gameState { waiting, ready, start, checkpointOne, checkpointTwo, checkpointThree, checkpointFour, finish };

enum rivalGameState { rivalStart, rivalCheckpointOne, rivalCheckpointTwo, rivalCheckpointThree, rivalCheckpointFour, rivalFinish};


void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\Users\Bethany\\Downloads\\Assignment Three Upload\\Assignment Three Upload\\media");

	/**** Set up your scene here ****/
	int screenHeight = myEngine->GetHeight();
	float spriteX = 15.0f;
	float spriteY = 110.0f;

	gameState currentGameState = waiting;

	rivalGameState currentRivalState = rivalStart;

	string gameStateTextArray[8] = { "waiting", "ready", "start", "checkpointOne", "checkpointTwo", "checkpointThree", "checkpointFour", "finish" };  //String array to match game state array for display.

	ISprite* uiBackdrop;                                                                            //Loading in assets depending on arrays of set positions.
	uiBackdrop = myEngine->CreateSprite("ui_backdrop.png", spriteX, (screenHeight - spriteY));

	float myFontSize = 40.0f;
	float stateFontSize = 20.0f;

	IFont* myFont = myEngine->LoadFont("Comic Sans MS", myFontSize);
	IFont* stateFont = myEngine->LoadFont("Comic Sans MS", stateFontSize);

	IMesh* carMesh = myEngine->LoadMesh("race2.x");
	IModel* car = carMesh->CreateModel(-4.0f, 0.0f, -10.0f);

	IMesh* rivalMesh = myEngine->LoadMesh("race2.x");
	IModel* rival = rivalMesh->CreateModel(4.0f, 0.0f, -10.0f);

	IMesh* dummyMesh = myEngine->LoadMesh("dummy.x");
	IModel* dummy[kNumDummy];

	IMesh* skyMesh = myEngine->LoadMesh("skybox 07.x");
	IModel* sky = skyMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IMesh* groundMesh = myEngine->LoadMesh("ground.x");
	IModel* ground = groundMesh->CreateModel();

	IMesh* checkpointMesh = myEngine->LoadMesh("checkpoint.x");
	IModel* checkpoint[kNumCheckpoint];

	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IModel* isle[kNumIsle];

	IMesh* aiDummyMesh = myEngine->LoadMesh("dummy.x");
	IModel* aiDummy[kNumAIDummy];

	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IModel* wall[kNumWalls];

	IMesh * borderMesh = myEngine->LoadMesh("skyscraper02.x");
	IModel* leftBorder[kNumSideBorders];
	IModel* rightBorder[kNumSideBorders];
	IModel* topBorder[kNumTopBorders];
	IModel* bottomBorder[kNumTopBorders];

	float borderScaleY = 0.1;                //Scaling the borders of the track, as original models are much larger than required.
	float borderScaleZ = 0.07;
	float borderRotation = 90.0f;

	IMesh* tankOneMesh = myEngine->LoadMesh("tanksmall1.x");
	IModel* tankOne[kNumTanks];

	IMesh* tankTwoMesh = myEngine->LoadMesh("tanksmall2.x");
	IModel* tankTwo[kNumSecondTanks];

	IMesh* crossMesh = myEngine->LoadMesh("Cross.x");
	IModel* cross[kNumCrosses];

	ICamera* myCamera = myEngine->CreateCamera(kManual);
	myCamera->AttachToParent(car);           //Camera is immediately attached to the player model to create a chase camera. 

	int mouseMoveX = myEngine->GetMouseMovementX();          //The camera has 'dual' controls; it can be controlled by the mouse or the keyboard.
	int mouseMoveY = myEngine->GetMouseMovementY();     

	float camPosition[3] = { 0.0f, 0.0f, 0.0f };             //For resetting and placing the camera, positions are placed in an array.
	float fpCamPosition[3] = { 0.0f, 5.0f, 1.0f };

	camPosition[0] = myCamera->GetLocalX();
	camPosition[1] = myCamera->GetLocalY();
	camPosition[2] = myCamera->GetLocalZ();

	float cameraSpeed = 10.0f;
	float cameraLimit = 15.0f;                  //The camera cannot rotate more than 15 in any direction to prevent clipping through the ground and keeping the game playable.
	float cameraGround = 0.5f;

	bool firstPerson = false;                   //First person camera initially set to false to the camera starts in third person.
	bool mouseCamera = false;					//The mouse controls are also set to false, so the game starts with keyboard camera control.

	bool winning = false;

	float countdown = 5.0f;                     //The countdown for starting the race.

	//Asset positions.

	float isleXPos[kNumIsle] = { -21.0f, 19.0f, 19.0f, -21.0f,  79.0f, 119.0f, 79.0f,  119.0f,-21.0f, 19.0f, 119.0f, 79.0f,-21.0f, 19.0f, 119.0f, 79.0f };
	float isleZPos[kNumIsle] = {  80.0f, 80.0f, 106.0f, 106.0f, 80.0f, 80.0f,  106.0f, 80.0f,  10.0f, 10.0f, 10.0f,  10.0f, 183.0f, 183.0f , 183.0f, 183.0f };

	float checkpointXPos[kNumCheckpoint] = { 0.0f, 0.0f, 100.0f, 100.0f };
	float checkpointZPos[kNumCheckpoint] = { 0.0f, 200.0f, 200.0f, 0.0f };

	float crossXPos[kNumCrosses] = { 0.0f, 0.0f, 100.0f, 100.0f };
	float crossYPos[kNumCrosses] = { -10.0f, -10.0f, -10.0f, -10.0f };
	float crossZPos [kNumCrosses] = { 0.0f, 200.0f, 200.0f, 0.0f };

	float wallXPos[kNumWalls] = { -21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f, -21.0f, 19.0f, 79.0f, 119.0f, -21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f,-21.0f, 19.0f, 79.0f, 119.0f,-21.0f,  19.0f,  79.0f,  119.0f,-21.0f,  19.0f,  79.0f,  119.0f, -21.0f, 19.0f,  79.0f,  119.0f,-21.0f,  19.0f,  79.0f,  119.0f, -21.0f, 19.0f, 79.0f, 119.0f, -21.0f, 19.0f, 79.0f, 119.0f, -21.0f, 19.0f, 79.0f, 119.0f,  -21.0f, 19.0f, 79.0f, 119.0f,  -21.0f, 19.0f , 79.0f,  119.0f };
	float wallZPos[kNumWalls] = { 17.5f, 17.5f, 17.5f, 17.5f,  27.0f, 27.0f, 27.0f, 27.0f,   36.5f, 36.5f, 36.5f, 36.5f,   46.0f, 46.0f, 46.0f, 46.0f,  55.5f, 55.5f, 55.5f, 55.5f,  65.0f, 65.0f, 65.0f, 65.0f,  74.5f, 74.5f, 74.5f, 74.5f,  84.0f, 84.0f, 84.0f, 84.0f,  93.5f, 93.5f, 93.5f, 93.5f,  103.0f, 103.0f, 103.0f, 103.0f, 112.5f, 112.5f, 112.5f, 112.5f, 122.0f, 122.0f, 122.0f, 122.0f, 131.5f, 131.5f, 131.5f, 131.5f,  141.0f, 141.0f, 141.0f, 141.0f, 150.5f, 150.5f, 150.5f, 150.5f, 160.0f, 160.0f, 160.0f, 160.0f, 169.5f, 169.5f, 169.5f, 169.5f, 179.0f, 179.0f, 179.0f, 179.0f };

	float tankXPos[kNumTanks] = { -20.0f, 20.0f, 80.0f, 140.0f, -20.0f, 20.0f, 80.0f, 140.0f };
	float tankZPos[kNumTanks] = { 240.0f, 300.0f, 300.0f, 260.0f, -40.0f, -100.0f, -100.0f, -60.0f };

	float tank2XPos[kNumSecondTanks] = { 5.5f, 1.0f, -1.0f, -5.0f, 2.0f  };
	float tank2YPos[kNumSecondTanks] = { -5.0f, -5.0, -5.0f, -5.0f, -5.0f };
	float tank2ZPos[kNumSecondTanks] = { 80.0f, 100.0f, 120.0f, 130.0f, 150.0f };

	float tank2Rotation = -20.0f;

	float aiDummyX[kNumAIDummy] = { -15.0f, 0.0f,  10.0f,  50.0f,  90.0f,  100.0f, 100.0f, 90.0f, 50.0f, 10.0f, 0.0f};
	float aiDummyZ[kNumAIDummy] = { 100.0f, 200.0f,240.0f, 270.0f, 240.0f, 200.0f,  0.0f, -40.0f,-70.0f,-40.0f, 0.0f};

	float sideBorderStartX = -50.0f;
	float borderStartY = -3.0f;
	float sideBorderStartZ = -140.0f;
	float currentSideBorder = 0.0f;
	float topBorderStartX = -15.0f;
	float topBorderStartZ = -175.0f;
	float currentTopBorder = 0.0f;

	int playerCurrentCheckpoint = 1;             //Checkpoint counters for the race positions of the rival/player car.
	int rivalCurrentCheckpoint = 1;
	int currentCheckpoint = 1;

	sMovementVector momentum = { 0.0f, 0.0f };   //Movement values, including maximum speed, thrust, and boost counters.
	float thrust = 0.0f;
	float thrustMax = 100.0f;
	float thrustMin = -50.0f;
	float overheatThrustMax = 50.0f;
	int thrustTimer = 3;
	float acceleration = 0.01f;
	float rotationSpeed = 0.1f;

	float minHeight = 0.0f;                      //Values for car hover.
	float maxHeight = 1.0f;
	float currentHeight;
	float bounceSpeed = 0.001f;

	float currentTilt = 0.0f;                    //Values for car tilt.
	float tiltSpeed = 0.02f;
	float maxTilt = 8.0f;
	float minTilt = 0.0f;
	float reverseMaxTilt = -10.0f;

	int maxLaps = 6;                             //Maximum number of laps, (can be adjusted for testing and time purposes).

	float rivalTime = 0.0f;                      //Values to track lap time.
	float carTime = 0.0f;
	float currentLapTime = 0.0f;
	float rivalLapTime = 0.0f;

	bool lapTimeTrigger = false;                 //Because the cars start behind the first checkpoint and the lap time should only trigger following a lap, the bool checks when the lap should trigger.

	float currentLean = 0.0f;                    //Values for the car's lean.
	float leanSpeed = 0.02;
	float maxLean = 12.0f;

	float crossTimer[kNumCrosses] = { 0.0f };    //Values for the cross rising upon checkpoint collision and lowering after a few seconds.
	bool isUp[kNumCrosses] = { false };
	float crossHeight = 10.0f;

	float carStart[3] = { car->GetX(),car->GetY(),car->GetZ() };                   //Start positions of both cars tracked for the reset.
	float rivalStartPos[3] = { rival->GetX(),rival->GetY(),rival->GetZ() };

	int currentAIPath = 0;                       //Values for AI car movement.
	int currentPlayerPath = 0;
	float rivalSpeed = 0.05f;

	bool thrusting = false;                      //Values for the thrust mechanic.
	bool canThrust = true;
	float thrustCounter = 0.0f;
	float overHeatingCounter = 0.0f;
	int overHeatingTimer = 5;

	int lapCounter = 1;                          //Counter that rises upon every completed lap, bar the final one.

	for (int i = 0; i < kNumIsle; i++)                                                                   //For loops to load in most assets.
	{
		isle[i] = isleMesh->CreateModel(isleXPos[i], 0.0f, isleZPos[i]);
	}
	for (int i = 0; i < kNumCheckpoint; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(checkpointXPos[i], 0.0f, checkpointZPos[i]);
	}
	for (int i = 0; i < kNumCrosses; i++)
	{
		cross[i] = crossMesh->CreateModel(crossXPos[i], crossYPos[i], crossZPos[i]);
	}
	for (int i = 0; i < kNumWalls; i++)
	{
		wall[i] = wallMesh->CreateModel(wallXPos[i], 0.0f, wallZPos[i]);
	}
	for (int i = 0; i < kNumTanks; i++)
	{
		tankOne[i] = tankOneMesh->CreateModel(tankXPos[i], 0.0f, tankZPos[i]);
	}
	for (int i = 0; i < kNumSecondTanks; i++)
	{
		tankTwo[i] = tankTwoMesh->CreateModel(tank2XPos[i], tank2YPos[i], tank2ZPos[i]);
		tankTwo[i]->RotateLocalZ(tank2Rotation);
	}
	for (int i = 0; i < kNumDummy; i++)
	{
		dummy[i] = dummyMesh->CreateModel();
	}
	for (int i = 0; i < kNumAIDummy; i++)
	{
		aiDummy[i] = aiDummyMesh->CreateModel(aiDummyX[i], 0.0f, aiDummyZ[i]);
	}

	for (int i = 0; i < kNumSideBorders; i++)
	{
		leftBorder[i] = borderMesh->CreateModel(sideBorderStartX, borderStartY, sideBorderStartZ + currentSideBorder);
		rightBorder[i] = borderMesh->CreateModel(sideBorderStartX + 210, borderStartY, sideBorderStartZ + currentSideBorder);

		currentSideBorder += 74.2f;

		leftBorder[i]->ScaleY(borderScaleY);
		leftBorder[i]->ScaleZ(borderScaleZ);
		leftBorder[i]->RotateY(borderRotation);
		
		rightBorder[i]->ScaleY(borderScaleY);
		rightBorder[i]->ScaleZ(borderScaleZ);
		rightBorder[i]->RotateY(borderRotation);
	}

	for (int i = 0; i < kNumTopBorders; i++)
	{
		topBorder[i] = borderMesh->CreateModel(topBorderStartX + currentTopBorder, borderStartY, topBorderStartZ);
		bottomBorder[i] = borderMesh->CreateModel(topBorderStartX + currentTopBorder, borderStartY, topBorderStartZ  + 515);

		currentTopBorder += 74.2f;

		topBorder[i]->ScaleY(borderScaleY);
		topBorder[i]->ScaleZ(borderScaleZ);

		bottomBorder[i]->ScaleY(borderScaleY);
		bottomBorder[i]->ScaleZ(borderScaleZ);
	}

	rival->SetSkin("chekon.jpg");                                      //Changing the skin of the rival.
	rival->LookAt(aiDummy[currentAIPath]);                             //Looking at the next dummy in its pathing for movement.

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene0
		myEngine->DrawScene();
		float gameTime = myEngine->Timer();                            //Timer that most of the game is based on.

		/**** Update your scene each frame here ****/

		stringstream gameStateText;                                                             //Adding text to the screen in a certain position to display the current game state.
		gameStateText << "Game State: " << gameStateTextArray[currentGameState];
		stateFont->Draw(gameStateText.str(), fontPos, screenHeight - stateFontPos, kWhite);

		if (myEngine->KeyHit(startKey))
		{
			currentGameState = ready;
		}

		if (currentGameState == ready)
		{
			countdown -= gameTime;

			if (countdown >= 4)
			{
				myFont->Draw("3", fontPos, screenHeight - fontPos, kWhite);
			}

			else if (countdown >= 3)
			{
				myFont->Draw("2", fontPos, screenHeight - fontPos, kWhite);
			}

			else if (countdown >= 2)
			{
				myFont->Draw("1", fontPos, screenHeight - fontPos, kWhite);
			}

			else if (countdown >= 1)
			{
				myFont->Draw("GO", fontPos, screenHeight - fontPos, kGreen);
			}

			else
			{
				currentGameState = start;
			}
		}

		if (currentGameState == waiting)
		{
			myFont->Draw("Press Space to Start", fontPos, screenHeight - fontPos, kWhite);             //Telling the player to start the game in ONE game state. 
		}

		if (currentGameState != ready && currentGameState != waiting)                                  //After the game has started...
		{

			if (myEngine->KeyHeld(anticlockwise))                                //Controlling the car.
			{
				car->RotateY(-rotationSpeed);
				if (currentLean < maxLean)
				{
					car->RotateLocalZ(leanSpeed);
					currentLean += leanSpeed;
				}
			}
			else if (myEngine->KeyHeld(clockwise))
			{
				car->RotateY(rotationSpeed);
				if (currentLean > -maxLean)
				{
					car->RotateLocalZ(-leanSpeed);
					currentLean -= leanSpeed;
				}
			}
			else
			{
				if (currentLean > 0.0f)
				{
					car->RotateLocalZ(-leanSpeed);
					currentLean -= leanSpeed;
				}
				else
				{
					car->RotateLocalZ(leanSpeed);
					currentLean += leanSpeed;
				}
			}



			if (myEngine->KeyHeld(kForward))
			{
				if (thrust <= thrustMax)
				{
					thrust += acceleration;

				}
				else
				{
					thrust = thrustMax;
				}
			}
			else if (myEngine->KeyHeld(back))
			{
				if (thrust >= thrustMin)
				{
					thrust -= acceleration * 2;          //Doubles the acceleration going backwards to counter momentum.
				}
				else
				{
					thrust = thrustMin;
				}
			}
			else
			{
				if (thrust > 0.0f)
				{
					thrust -= acceleration;
				}
				else if (thrust < 0.0f)
				{
					thrust += acceleration;
				}
			}

			if (myEngine->KeyHeld(boost))                       //The boost works based on a boolean and the game timer.
			{
				if (canThrust)
				{
					if (thrustCounter < thrustTimer)           //If the thrust is active for less than three seconds, it will continue.
					{
						thrust = thrustMax * 2;                //The maximum thrust is doubled while the car can boost.
						thrustCounter += gameTime;

					}
					else
					{

						canThrust = false;                     //Otherwise, 'canThrust' is false and the maximum thrust is reset to 100.
						thrust = thrustMax; 
					}
				}
			}

			if (!canThrust)                                    //Five second 'cooldown' for the boost function.
			{
				if (overHeatingCounter < overHeatingTimer)
				{
					overHeatingCounter += gameTime;	
				}
				else
				{
					canThrust = true;                          //When the boost has passed its cooldown, it is reset and usable again.
					overHeatingCounter = 0.0f;
					thrustCounter = 0.0f;
				}
			}

			rival->LookAt(aiDummy[currentAIPath]);             //Rival car movement.
			rival->MoveLocalZ(rivalSpeed);

			if (aiDummyCollision(rival, aiDummy[currentAIPath]))
			{
				currentAIPath++;
			}

			if (currentAIPath >= kNumAIDummy)
			{
				currentAIPath = 0;
			}

			if (playerDummyCollision(car, aiDummy[currentPlayerPath]))
			{
				currentPlayerPath++;
			}

			if (currentPlayerPath >= kNumAIDummy)
			{
				currentPlayerPath = 0;
			}

			if (aiDummyCollision(car, rival))
			{
				momentum.xVector = -momentum.xVector;
				momentum.zVector = -momentum.zVector;
				hpCounter -= damage * 5;                   //Five health points are deducted for every collision with the rival car.
			}

			directionCalculation(car, thrust, momentum, gameTime);           
			car->Move(momentum.xVector * gameTime, 0.0f, momentum.zVector * gameTime);

			float oldX = car->GetX();
			float oldY = car->GetY();
			float oldZ = car->GetZ();

			


			for (int i = 0; i < kNumWalls; i++)                   //Wall Collisions
			{
				if (sphereToBox(car, wall[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}


			for (int i = 0; i < kNumIsle; i++)                   //Isle Collisions
			{
				if (sphereToBox(car, wall[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			for (int i = 0; i < kNumCheckpoint; i++)              //Checkpoint Post Collisions
			{
				if (sphereToSphere(car, checkpoint[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			for (int i = 0; i < kNumTanks; i++)                   //Corner Tank Collisions
			{
				if (tankSphereToSphere(car, tankOne[i]))
				{   
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			for (int i = 0; i < kNumSecondTanks; i++)             //Sunken Tank Collisions
			{
				if (tankSphereToSphere(car, tankTwo[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			for (int i = 0; i < kNumSideBorders; i++)                   //Border Collisions
			{
				if (sideBorderCollision(car, leftBorder[i]) || sideBorderCollision(car, rightBorder[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			for (int i = 0; i < kNumTopBorders; i++)                   
			{
				if (topBorderCollision(car, topBorder[i]) || topBorderCollision(car, bottomBorder[i]))
				{
					car->SetPosition(oldX, oldY, oldZ);
					momentum.xVector = -momentum.xVector;
					momentum.zVector = -momentum.zVector;
					hpCounter -= damage;
				}
			}

			if (currentGameState != finish)                                                         //The following checks the cars' positions for comparison (race positions).
				                                                                                    //It also includes the cross code, so the cross will rise (appear) once a 'collision' has happened.
			{
				if (pointToBox(car, checkpoint[1]))
				{
					if (currentGameState == start  || currentGameState == checkpointFour)
					{
						currentCheckpoint = 2;
						currentGameState = checkpointOne;
						cross[1]->SetY(crossHeight);
						playerCurrentCheckpoint++;
					}
				}

				if (pointToBox(car, checkpoint[2]))
				{
					if (currentGameState == checkpointOne)
					{
						currentCheckpoint = 3;
						currentGameState = checkpointTwo;
						cross[2]->SetY(crossHeight);
						playerCurrentCheckpoint++;
					}
				}

				if (pointToBox(car, checkpoint[3]))
				{
					if (currentGameState == checkpointTwo)
					{
						currentCheckpoint = 0;
						currentGameState = checkpointThree;
						lapTimeTrigger = true;
						cross[3]->SetY(crossHeight);
						playerCurrentCheckpoint++;
					}

				}


				if (pointToBox(car, checkpoint[0]))
				{			
					if (currentGameState == checkpointThree)
					{
						currentCheckpoint = 1;
						currentGameState = checkpointFour;
						cross[0]->SetY(crossHeight);
						lapCounter++;
						playerCurrentCheckpoint++;

						for (int i = 0; i < kNumCrosses; i++)
						{
							crossTimer[i] = 0.0f;
						}
					}
				}

				if ((currentGameState == checkpointThree) && (lapCounter == maxLaps))
				{
					currentGameState = finish;
					cross[0]->SetY(crossHeight);
				}

				if (pointToBox(rival, checkpoint[1]))
				{
					if ((currentRivalState == rivalStart) || (currentRivalState == rivalCheckpointFour))
					{
						currentRivalState = rivalCheckpointOne;
						rivalCurrentCheckpoint++;
					}
				}
				if (pointToBox(rival, checkpoint[2]))
				{
					if (currentRivalState == rivalCheckpointOne)
					{
						currentRivalState = rivalCheckpointTwo;
						rivalCurrentCheckpoint++;
					}
				}
				if (pointToBox(rival, checkpoint[3]))
				{
					if (currentRivalState == rivalCheckpointTwo)
					{
						currentRivalState = rivalCheckpointThree;
						rivalCurrentCheckpoint++;
					}
				}
				if (pointToBox(rival, checkpoint[0]))
				{
					if ( currentRivalState == rivalCheckpointThree)
					{
						currentRivalState = rivalCheckpointFour;
						rivalCurrentCheckpoint++;
					}
				}

				if (currentGameState == checkpointOne)                                                  //Updates the status of the player going through the course as they hit checkpoints.
				{
					myFont->Draw("Stage One Complete", fontPos, screenHeight - fontPos, kWhite);
				}

				if (currentGameState == checkpointTwo)
				{
					myFont->Draw("Stage Two Complete", fontPos, screenHeight - fontPos, kWhite);
				}

				if (currentGameState == checkpointThree)
				{
					myFont->Draw("Stage Three Complete", fontPos, screenHeight - fontPos, kWhite);
				}

				if (currentGameState == checkpointFour)
				{
					myFont->Draw("Stage Four Complete", fontPos, screenHeight - fontPos, kWhite);
				}

			}
			else
			{
				myFont->Draw("Race Over", fontPos, screenHeight - fontPos, kYellow);
			}


			for (int i = 0; i < kNumCrosses; i++)
			{
				if (cross[i]->GetY() >= crossHeight)
				{
					crossTimer[i] += gameTime;
					isUp[i] = true;
				}

				if (crossTimer[i] >= 2.0f && isUp[i] == true)                     //The cross falls after a certain amount of time, so it can come back up for every new lap.
				{
					cross[i]->SetY(-crossHeight);
					isUp[i] = false;
				}
			}
		}

		car->MoveY(bounceSpeed);                  //Car hovering.
		rival->MoveY(bounceSpeed);
		if (firstPerson == false)
		{
			myCamera->MoveY(-bounceSpeed);
		}
		

		currentHeight = car->GetY();

		if (currentHeight <= minHeight)
		{
			bounceSpeed = -bounceSpeed;
		}

		if (currentHeight >= maxHeight)
		{
			bounceSpeed = -bounceSpeed;
		}

		if (myEngine->KeyHeld(kForward) && currentTilt <= maxTilt)         //Car tilting.
		{
			car->RotateLocalX(-tiltSpeed);
			myCamera->RotateLocalX(tiltSpeed);
			currentTilt += tiltSpeed;
		}
		else if (currentTilt >= minTilt)
		{
			car->RotateLocalX(tiltSpeed);
			myCamera->RotateLocalX(-tiltSpeed);
			currentTilt -= tiltSpeed;
		}

		if (myEngine->KeyHit(quitKey))                   //When escape is hit, the game closes. 
		{
			myEngine->Stop();
		}

		float thrustText = thrust;                       //Displaying the player car speed.
		stringstream speedText;
		speedText << "Speed: " << thrustText;

		if (thrust >= thrustMax)
		{
			myFont->Draw(speedText.str(), textPositionX, speedY, kRed);        //If the maximum speed is above 100, the text turns red, to indicate highest speed or boost being active.
		}

		else
		{
			myFont->Draw(speedText.str(), textPositionX, speedY);
		}

		int hpText = hpCounter;                           //Displaying the player car health.
		stringstream hpBar;
		hpBar << "Health: " << hpText;

		if (currentGameState != finish)
		{
			if (hpCounter <= 0)
			{
				myFont->Draw("Race Over", fontPos, screenHeight - fontPos, kYellow);          //If the counter hits 0, the game is set to its finish state and is over.
				currentGameState = finish;
			}

			if (hpCounter <= lowHealth)                                                       //When player health gets very low, it is further indicated by turning the text red.
			{
				myFont->Draw(hpBar.str(), textPositionX, hpY, kRed);
			}
			else if (hpCounter <= midHealth)                                                   //When player health gets low, it is further indicated by turning the text yellow.
			{
				myFont->Draw(hpBar.str(), textPositionX, hpY, kYellow);
			}
			else
			{
				myFont->Draw(hpBar.str(), textPositionX, hpY, kGreen);                         //While player health is fairly high, the text remains green. 
			}
		}

		int lapText = lapCounter;                            //Displays current lap while the game is not in its finish state.
		stringstream lapNumber;
		lapNumber << "Lap: " << lapText << " /6";

		

		if (currentGameState != finish)
		{
			myFont->Draw(lapNumber.str(), textPositionX, lapY, kBlue); 
		}

		if (rivalCurrentCheckpoint < playerCurrentCheckpoint)                 //Checking the position of the cars against each other to establish who is further ahead.
		{
				myFont->Draw("Player in first place.", textPositionX, positionY, kRed);
				winning = true;
		}
		else if (rivalCurrentCheckpoint > playerCurrentCheckpoint)
		{
				myFont->Draw("Rival in first place.", textPositionX, positionY, kBlue);
				winning = false;
		}
		else
		{
			if (racePosition(car, rival, checkpoint[currentCheckpoint]))
			{
				myFont->Draw("Player in first place.", textPositionX, positionY, kRed);
				winning = true;
			}
			else
			{
				myFont->Draw("Rival in first place.", textPositionX, positionY, kBlue);
				winning = false;
			}
		}

		if (currentGameState != finish)                        //As long as the game is still being played, the time it takes for the cars to move around the map is tracked.
		{
			carTime += gameTime;
			rivalTime += gameTime;
		
			if (pointToBox(car, checkpoint[0]))
			{
				currentLapTime = carTime;
			}

			if (pointToBox(rival, checkpoint[0]))
			{
				rivalLapTime = rivalTime;
			}

			float carTimeText = carTime;                         //Displaying the times, updating every time the fourth checkpoint is hit.
			stringstream carLap;
			carLap << "Player Time: " << currentLapTime;

			float rivalTimeText = rivalTime;
			stringstream rivalLap;
			rivalLap << "Rival Time: " << rivalLapTime;

			if (lapTimeTrigger == true && lapCounter > 1)
			{
				myFont->Draw(carLap.str(), lapTimeX, playerLapTimeY, kGreen);
				myFont->Draw(rivalLap.str(), lapTimeX, rivalLapTimeY, kGreen);
			}
		}

		if (winning == false && currentGameState == finish)           //"Winning" conditions - depending on what conditions are met, a winner is displayed at the end of the race.
		{
			myFont->Draw("Rival Wins!", winnerX, winnerY, kRed);
		}
		else if (winning == true  && currentGameState == finish)
		{
			myFont->Draw("Player Wins!", winnerX, winnerY, kBlue);
		}
		else if (hpCounter == 0)
		{
			myFont->Draw("Rival Wins!", winnerX, winnerY, kRed);
		}

		


		//Camera Controls

		if (mouseCamera == false)                           //If the mouse controls are false, the camera is controlled by the keyboard.
		{
			if (myCamera->GetY() <= cameraLimit)
			{
				if (myEngine->KeyHeld(cameraUp))
				{
					myCamera->RotateX(-gameTime * cameraSpeed);
				}
			}

			if (myCamera->GetY() >= cameraGround)
			{
				if (myEngine->KeyHeld(cameraDown))
				{
					myCamera->RotateX(gameTime * cameraSpeed);
				}
			}

			if (myCamera->GetLocalX() <= cameraLimit)
			{
				if (myEngine->KeyHeld(cameraRight))
				{
					myCamera->RotateY(gameTime * cameraSpeed);
				}
			}

			if (myCamera->GetLocalX() >= -cameraLimit)
			{
				if (myEngine->KeyHeld(cameraLeft))
				{
					myCamera->RotateY(-gameTime * cameraSpeed);
				}
			}
		}

		if (myEngine->KeyHit(cameraReset))                 //Resetting the camera moves it to its original position and resets the controls back to keyboard.
		{
			firstPerson = false;
			mouseCamera = false;
			myCamera->SetLocalPosition(camPosition[0], camPosition[1], camPosition[2]);
			myCamera->ResetOrientation();
		}

		if (myEngine->KeyHit(fpCamera))                    //First person sets the camera to the front of the car.
		{
			firstPerson = true;
			myCamera->SetLocalPosition(fpCamPosition[0], fpCamPosition[1], fpCamPosition[2]);
		}

		if (mouseCamera == true)                           //Controlling the camera via the mouse.
		{
			myEngine->StartMouseCapture();

			int mouseXMovement = myEngine->GetMouseMovementX();
			int mouseYMovement = myEngine->GetMouseMovementY();
			
			if (myCamera->GetY() <= cameraLimit)
			{
					myCamera->RotateY(gameTime * mouseXMovement);
			}

			if (myCamera->GetY() >= cameraLimit)
			{
				myCamera->RotateY(-gameTime * mouseXMovement);
			}

			if (myCamera->GetX() <= cameraLimit)
			{
				myCamera->RotateX(gameTime * mouseYMovement);
			}

			if (myCamera->GetX() >= cameraLimit)
			{
				myCamera->RotateX(-gameTime * mouseYMovement);
			}

		}

		if (myEngine->KeyHit(otherCamera))                  //Pressing the other camera button disables keyboard control and fully enables mouse control.
		{
			mouseCamera = true;

		}

		//Restart button

		if (myEngine->KeyHit(restart))                      //All these variables get reset to their original states when the restart button is pressed.
		{
			car->SetPosition(carStart[0], carStart[1], carStart[2]);
			car->ResetOrientation();
			rival->SetPosition(rivalStartPos[0], rivalStartPos[1], rivalStartPos[2]);
			rival->ResetOrientation();

			thrust = 0.0f;
			hpCounter = 100;
			countdown = 5.0f;
			currentAIPath = 0;
			momentum.xVector = 0.0f;
			momentum.zVector = 0.0f;
			lapCounter = 0;

			for (int i = 0; i < kNumCrosses; i++)
			{
				crossTimer[i] = 0.0f;
			}

			currentGameState = waiting;
			lapTimeTrigger = false;


		}



	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}


void directionCalculation(IModel* hovercar, float thrust, sMovementVector& momentum, float frameTime)
{
	sMovementVector dragVect = { 0.0F, 0.0F };
	sMovementVector dragNormVect = { 0.0F, 0.0F };
	sMovementVector facingVect;
	sMovementVector normFacingVect;
	float drag = -1.0f;
	float hoverMatrix[4][4];

	hovercar->GetMatrix(&hoverMatrix[0][0]);

	facingVect.xVector = hoverMatrix[2][0];
	facingVect.zVector = hoverMatrix[2][2];
	float magnitude = sqrt((facingVect.xVector * facingVect.xVector) + (facingVect.zVector * facingVect.zVector));

	normFacingVect.xVector = facingVect.xVector / magnitude;
	normFacingVect.zVector = facingVect.zVector / magnitude;

	normFacingVect.xVector *= thrust;
	normFacingVect.zVector *= thrust;

	dragVect.xVector = momentum.xVector * drag;
	dragVect.zVector = momentum.zVector * drag;

	momentum.xVector += (normFacingVect.xVector * frameTime) + (dragVect.xVector * frameTime);
	momentum.zVector += (normFacingVect.zVector * frameTime) + (dragVect.zVector * frameTime);
}

bool sphereToBox(IModel* car, IModel* wall)
{
	float wallLength = 4.75;
	
	float wallMinX = wall->GetX() - wallLength;
	float wallMaxX = wall->GetX() + wallLength;

	float wallMinZ = wall->GetZ() - wallLength;
	float wallMaxZ = wall->GetZ() + wallLength;

	if (car->GetX() >= wallMinX && car->GetX() <= wallMaxX && car->GetZ() >= wallMinZ && car->GetZ() <= wallMaxZ)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool pointToBox(IModel* car, IModel* checkpoint)
{
	float checkpointDist = 7.0f;
	float checkpointWidth = 1.0f;
	
	float checkpointMinX = checkpoint->GetX() - checkpointDist;
	float checkpointMaxX = checkpoint->GetX() + checkpointDist;

	float checkpointMinZ = checkpoint->GetZ() - checkpointWidth;
	float checkpointMaxZ = checkpoint->GetZ() + checkpointWidth;

	if (car->GetX() >= checkpointMinX && car->GetX() <= checkpointMaxX && car->GetZ() >= checkpointMinZ && car->GetZ() <= checkpointMaxZ)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool sphereToSphere(IModel* car, IModel* checkpoint)
{
	float xVector;
	float zVector;
	float distance;

	float checkpointLeg = 0.2f;
	float checkpointDistance = 8.0f;
	float dummyRadius = 2.0f;

	xVector = (checkpoint->GetX() - checkpointDistance) - car->GetX();         //Vector between two models on X-axis.
	zVector = checkpoint->GetZ() - car->GetZ();                                //Vector between two models on Z-axis.

	distance = sqrt((xVector * xVector) + (zVector * zVector));                //Distance between the two, (if distance is too small, a collision occurs).

	if (distance < checkpointLeg + dummyRadius)
	{
		return true;
	}


	xVector = (checkpoint->GetX() + checkpointDistance) - car->GetX();
	zVector = checkpoint->GetZ() - car->GetZ();

	distance = sqrt((xVector * xVector) + (zVector * zVector));

	if (distance < checkpointLeg + dummyRadius)
	{
		return true;
	}


	 
	return false;

}

bool tankSphereToSphere(IModel* car, IModel* tank)
{
	float sphereWidth = 4.0f;
	
	float xVector;
	float zVector;
	float distance;

	xVector = (tank->GetX()) - car->GetX();										//Vector between two models on X-axis.
	zVector = tank->GetZ() - car->GetZ();										//Vector between two models on Z-axis.

	distance = sqrt((xVector * xVector) + (zVector * zVector));                 //Distance between the two, (if distance is too small, a collision occurs).

	if (distance < sphereWidth)
	{
		return true;
	}
	else
	{
		return false;
	}


}

bool aiDummyCollision(IModel* aICar, IModel* dummy)
{
	float xVector;
	float zVector;
	float distance;

	xVector = dummy->GetX() - aICar->GetX();									//Vector between two models on X-axis.
	zVector = dummy->GetZ() - aICar->GetZ();									//Vector between two models on Z-axis.

	distance = sqrt((xVector * xVector) + (zVector * zVector));                 //Distance between the two, (if distance is too small, a collision occurs).

	if (distance <= carRadius + aiCarRadius)
	{
		return true; //
	}
	else
	{
		return false;
	}
}

bool playerDummyCollision(IModel* car, IModel* dummy)
{
	float xVector;
	float zVector;
	float distance;

	xVector = dummy->GetX() - car->GetX();										//Vector between two models on X-axis.
	zVector = dummy->GetZ() - car->GetZ();									    //Vector between two models on Z-axis.

	distance = sqrt((xVector * xVector) + (zVector * zVector));                 //Distance between the two, (if distance is too small, a collision occurs).

	if (distance <= carRadius + aiDummyRadius)
	{
		return true; //
	}
	else
	{
		return false;
	}
}

bool sideBorderCollision(IModel* car, IModel* sideBarrier)
{
	float wallWidth = 2.0f;
	float wallLength = 70.0f;

	float wallMinX = sideBarrier->GetX() - wallWidth;
	float wallMaxX = sideBarrier->GetX() + wallWidth;

	float wallMinZ = sideBarrier->GetZ() - wallLength;
	float wallMaxZ = sideBarrier->GetZ() + wallLength;

	if (car->GetX() >= wallMinX && car->GetX() <= wallMaxX && car->GetZ() >= wallMinZ && car->GetZ() <= wallMaxZ)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool topBorderCollision(IModel* car, IModel* topBarrier)
{
	float wallWidth = 2.0f;
	float wallLength = 70.0f;
	
	float wallMinX = topBarrier->GetX() - wallLength;
	float wallMaxX = topBarrier->GetX() + wallLength;

	float wallMinZ = topBarrier->GetZ() - wallWidth;
	float wallMaxZ = topBarrier->GetZ() + wallWidth;

	if (car->GetX() >= wallMinX && car->GetX() <= wallMaxX && car->GetZ() >= wallMinZ && car->GetZ() <= wallMaxZ)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool racePosition(IModel* car, IModel* rival, IModel* dummy)
{
	float carXVector;
	float carZVector;
	float rivalXVector;
	float rivalZVector;
	float carDistance;
	float rivalDistance;

	carXVector = dummy->GetX() - car->GetX(); 
	carZVector = dummy->GetZ() - car->GetZ();

	rivalXVector = dummy->GetX() - rival->GetX();
	rivalZVector = dummy->GetZ() - rival->GetZ();

	carDistance = sqrt((carXVector * carXVector) + (carZVector * carZVector));
	rivalDistance = sqrt((rivalXVector * rivalXVector) + (rivalZVector * rivalZVector));

	if (carDistance < rivalDistance)
	{
		return true;
	}
	else
	{
		return false;
	}

}