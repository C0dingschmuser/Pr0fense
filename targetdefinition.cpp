#include "targetdefinition.h"

TargetDefinition::TargetDefinition()
{

}

TargetDefinition::TargetDefinition(int enemyType, int enemyEfficiency)
{
    this->enemyType = enemyType;
    this->enemyEfficiency = enemyEfficiency;
}
