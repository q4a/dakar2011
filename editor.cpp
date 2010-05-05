/****************************************************************
*                                                               *
*    Name: editor.cpp                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "editor.h"
#include "MyRoad.h"
#include "BigTerrain.h"
#include "gameplay.h"
#include "pools.h"
#include "wrappers.h"

#ifdef USE_EDITOR
static gui::IGUIStaticText* editorText = 0;
static int currentRoad = 0;
static int currentObj = 0;
static int currentItiner = ITINER_POOLID_OFFSET;
static char tmpRoadFileName[256] = "data/editor/tmproads.txt";
static char tmpObjFileName[256] = "data/editor/tmpobjects.txt";
#endif // USE_EDITOR

void initEditor(IGUIEnvironment* env)
{
#ifdef USE_EDITOR
    editorText = env->addStaticText(L"POS: ",
                        core::rect<int>(10,110,300,600),
                        false/*info_bg*/, true, 0, -1, true);
    editorText->setVisible(false);
#endif // USE_EDITOR
}

void editorSetVisible(bool vis)
{
#ifdef USE_EDITOR
    editorText->setVisible(vis);
#endif // USE_EDITOR
}

void updateEditor()
{
#ifdef USE_EDITOR
    core::stringw str;
    str = L"Help:\n0 - addPoint to road (LMB)\n1 - prev road\n2 - next road\n3 - new road\n4 - save roads\n5 - load roads\nF7 - remove last point\n\n";
    str += L"6 - add new obj (RMB)\n7 - prev obj\n8 - next obj\n9 - refresh object wire\nU - save obj\nF8 - remove last obj\n\n";
    str += L"Del - add new itiner\nIns - prev itiner\nHome - next itiner\nEnd - remove nearest obj\n\n";
    
    str += L"Road: ";
    str += currentRoad + 1;
    str += L"/";
    str += bigTerrain->getRoadList().size();
    str += L"\nLen: ";
    //printf("%d %d", currentRoad, bigTerrain->getRoadList().size() - 1);
    if (currentRoad >= 0 && currentRoad <= (int)bigTerrain->getRoadList().size() - 1)
        str += bigTerrain->getRoadList()[currentRoad]->getBasePoints().size();
    
    str += L"\n\n";
    str += L"Obj: ";
    str += getPoolNameFromId(currentObj);
    str += L" (";
    str += getPoolCategoryFromId(currentObj);
    str += L")\nIti: ";
    str += getItinerNameFromId(currentItiner);
    str += L"\nFix objs: ";
    str += bigTerrain->getObjectWrappers().size();
    editorText->setText(str.c_str());
#endif // USE_EDITOR
}

void actionEditor(int key)
{
#ifdef USE_EDITOR
    switch (key)
    {
		case irr::KEY_KEY_0:
            {
                printf("add new base point\n");
                if (currentRoad >= 0 && currentRoad <= (int)bigTerrain->getRoadList().size() - 1)
                {
                    bigTerrain->getRoadList()[currentRoad]->addBasePoint(vector3df(
                                                                        camera->getPosition().X,
                                                                        0.f,
                                                                        camera->getPosition().Z
                                                                        ));
                    bigTerrain->updateRoads();
                }
                break;
            }
		case irr::KEY_F7:
            {
                printf("remove last base point\n");
                if (currentRoad >= 0 && currentRoad <= (int)bigTerrain->getRoadList().size() - 1 &&
                    bigTerrain->getRoadList()[currentRoad]->getBasePoints().size() > 0)
                {
                    bigTerrain->getRoadList()[currentRoad]->getBasePoints().erase(bigTerrain->getRoadList()[currentRoad]->getBasePoints().size()-1);
                    bigTerrain->updateRoads();
                }
                break;
            }
		case irr::KEY_KEY_1:
            {
                printf("prev road\n");
                if (currentRoad <= 0)
                {
                    currentRoad = (int)bigTerrain->getRoadList().size() - 1;
                }
                else
                {
                    currentRoad--;
                }
                break;
            }
		case irr::KEY_KEY_2:
            {
                printf("next road\n");
                if (currentRoad >= (int)bigTerrain->getRoadList().size()-1 )
                {
                    currentRoad = 0;
                }
                else
                {
                    currentRoad++;
                }
                break;
            }
		case irr::KEY_KEY_3:
            {
                printf("new road\n");
                bigTerrain->addNewRoad();
                break;
            }
		case irr::KEY_KEY_4:
            {
                CMyRoad::saveRoads(tmpRoadFileName, bigTerrain->getRoadList());
                break;
            }
		case irr::KEY_KEY_5:
            {
                bigTerrain->loadRoads(tmpRoadFileName);
                bigTerrain->updateRoads();
                break;
            }
		case irr::KEY_KEY_6:
            {
                if (currentObj >= 0 && currentObj < getPoolsSize())
                {
                    SObjectWrapper* objectWrapper = new SObjectWrapper(bigTerrain);
                    objectWrapper->setPosition(camera->getPosition());
                    objectWrapper->setPool(currentObj);
                    bigTerrain->getObjectWrappers().push_back(objectWrapper);
                    objectWrapper->setVisible(true);
                }
                break;
            }
		case irr::KEY_F8:
            {
                if (currentObj >= 0 && currentObj < getPoolsSize() &&
                    bigTerrain->getObjectWrappers().size() > 0)
                {
                    if (bigTerrain->getObjectWrappers()[bigTerrain->getObjectWrappers().size()-1]->getPool() > ITINER_POOLID_OFFSET)
                    {
                        bigTerrain->removeActiveItinerPoint((SItinerPoint*)bigTerrain->getObjectWrappers()[bigTerrain->getObjectWrappers().size()-1]);
                    }
                    delete bigTerrain->getObjectWrappers()[bigTerrain->getObjectWrappers().size()-1];
                    bigTerrain->getObjectWrappers().erase(bigTerrain->getObjectWrappers().size()-1);
                }
                break;
            }
		case irr::KEY_KEY_7:
            {
                if (currentObj <= 0)
                {
                    currentObj = getPoolsSize() - 1;
                }
                else
                {
                    currentObj--;
                }
                break;
            }
		case irr::KEY_KEY_8:
            {
                if (currentObj >= getPoolsSize() - 1)
                {
                    currentObj = 0;
                }
                else
                {
                    currentObj++;
                }
                break;
            }
		case irr::KEY_KEY_9:
            {
                bigTerrain->updateObjectWire();
                break;
            }
		case irr::KEY_KEY_U:
            {
                bigTerrain->saveObjects(tmpObjFileName);
                break;
            }
		case irr::KEY_DELETE:
            {
                if (currentItiner >= ITINER_POOLID_OFFSET && currentItiner < ITINER_POOLID_OFFSET + getItinerTypesSize())
                {
                    SItinerPoint* objectWrapper = new SItinerPoint(bigTerrain);
                    objectWrapper->setPosition(camera->getPosition());
                    objectWrapper->setPool(currentItiner);
                    bigTerrain->getObjectWrappers().push_back(objectWrapper);
                    objectWrapper->setVisible(true);
                }
                break;
            }
		case irr::KEY_INSERT:
            {
                if (currentItiner <= 0)
                {
                    currentItiner = ITINER_POOLID_OFFSET + getItinerTypesSize() - 1;
                }
                else
                {
                    currentItiner--;
                }
                break;
            }
		case irr::KEY_HOME:
            {
                if (currentItiner >= ITINER_POOLID_OFFSET + getItinerTypesSize() - 1)
                {
                    currentItiner = ITINER_POOLID_OFFSET;
                }
                else
                {
                    currentItiner++;
                }
                break;
            }
		case irr::KEY_END:
            {
                if (currentObj >= 0 && currentObj < getPoolsSize() &&
                    bigTerrain->getObjectWrappers().size() > 0)
                {
                    float nearestDist = 50.f;
                    int nearestInd = bigTerrain->getObjectWrappers().size();
                    for (int i = 0; i < bigTerrain->getObjectWrappers().size(); i++)
                    {
                        float len = camera->getPosition().getDistanceFrom(bigTerrain->getObjectWrappers()[i]->getPosition());
                        if (len < nearestDist)
                        {
                            nearestInd = i;
                            nearestDist = len;
                        }
                    }
                    if (nearestInd == bigTerrain->getObjectWrappers().size()) break;
                    if (bigTerrain->getObjectWrappers()[nearestInd]->getPool() > ITINER_POOLID_OFFSET)
                    {
                        bigTerrain->removeActiveItinerPoint((SItinerPoint*)bigTerrain->getObjectWrappers()[nearestInd]);
                    }
                    delete bigTerrain->getObjectWrappers()[nearestInd];
                    bigTerrain->getObjectWrappers().erase(nearestInd);
                }
                break;
            }
        default:
            break;
    }
#endif // USE_EDITOR
}
/*
void initEditor()
{
#ifdef USE_EDITOR
#endif // USE_EDITOR
}
*/
