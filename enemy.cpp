#include "enemy.h"

Enemy::Enemy()
{
    isUsed = false;
    body = nullptr;
    physicsInitialized = false;
    newspeed = 0;
}

void Enemy::init(QRectF pos, double speed, int type, int cpos, int health, int path, int price)
{
    soonBanned = false;
    physicsInitialized = false;
    this->price = price;
    this->pos = pos;
    this->cpos = cpos;
    this->health = health;
    this->maxHealth = health;
    this->preHealth = health;
    this->speed = speed;
    this->ospeed = speed;
    this->type = type;
    this->path = path;
    newspeed = 0;
    isUsed = true;
    dir = 1; //rechts
}

void Enemy::initPhysics(b2World *world)
{
    b2BodyDef BodyDef;
    BodyDef.type = b2_dynamicBody;
    BodyDef.position.Set(Engine::numToBox2D(pos.x()+(pos.width()/2)),Engine::numToBox2D(pos.y()+(pos.height()/2)));
    BodyDef.angle = 0;
    body = world->CreateBody(&BodyDef);
    b2CircleShape circleShape;
    //b2PolygonShape polyShape;
    b2FixtureDef fixtureDef;
    //if(type == 7) { //wenn legende
        circleShape.m_p.Set(0,0);
        circleShape.m_radius = Engine::numToBox2D(pos.width() / 2);
        fixtureDef.shape = &circleShape;
    /*} else {
        b2Vec2 array[4];
        double x = Engine::numToBox2D(pos.x());
        double y = Engine::numToBox2D(pos.y());
        double w = Engine::numToBox2D(pos.width());
        double h = Engine::numToBox2D(pos.height());
        array[0].Set(-w/2,-h/2);
        array[1].Set(w/2,-h/2);
        array[2].Set(w/2,h/2);
        array[3].Set(-w/2,h/2);
        polyShape.Set(array, 4);
         //polyShape.SetAsBox(Engine::numToBox2D(pos.width()/2), Engine::numToBox2D(pos.height()/2));
         fixtureDef.shape = &polyShape;
    }*/
    fixtureDef.density = getDensity(type);
    fixtureDef.filter.categoryBits = ownCategoryBits;
    fixtureDef.filter.maskBits = collidingCategoryBits;
    body->CreateFixture(&fixtureDef);
    updatePos();
    physicsInitialized = true;
}

void Enemy::moveBy(double xv, double yv)
{
    if(!physicsInitialized || body == nullptr) return;
    //qDebug()<<body->GetWorld()->IsLocked();
    body->SetLinearVelocity(b2Vec2(xv*2,yv*2));
    updatePos();
    if(opacity<1) opacity += 0.0075;
}

void Enemy::moveTo(double x, double y)
{
    if(body != nullptr && physicsInitialized) {
        body->SetTransform(b2Vec2(Engine::numToBox2D(x+pos.width()/2),Engine::numToBox2D(y+pos.height()/2)),0);
        updatePos();
    }
}

void Enemy::setHealth(int health)
{
    this->health = health;
    this->maxHealth = health;
    this->preHealth = health;
}

void Enemy::setSpeed(double speed)
{
    this->speed = speed;
    this->ospeed = speed;
}

void Enemy::setStun(int stunAmount)
{
    this->stunned += stunAmount;
    this->maxStun = stunned;
}

void Enemy::setRepost(int repostAmount)
{
    this->repost += repostAmount;
    this->maxRepost = repostAmount;
}

void Enemy::reduceHealth(double amount)
{
    if(repost && !soonBanned && !poison) amount *= 1.5;
    health -= amount;
    updateWidth(0);
    flash = 10;
    if(health<0) health = 0;
}

void Enemy::free()
{
    flash = 0;
    blocked = false;
    body = nullptr;
    pos = QRectF();
    cpos = 0;
    health = 0;
    newspeed = 0;
    path = -1;
    isUsed = false;
    stunned = 0;
    maxStun = 0;
    repost = 0;
    maxRepost = 0;
    maxPoison = 0;
    poison = 0;
    poisonDmg = 1;
    animation = 0;
}

void Enemy::updatePos()
{
    if(!physicsInitialized||body==nullptr) return;
    this->pos = QRectF(Engine::numToNormal(body->GetPosition().x)-(pos.width()/2),
                       Engine::numToNormal(body->GetPosition().y)-(pos.height()/2),
                       pos.width(),
                       pos.height());
}

void Enemy::calcWidth()
{
    healthWidth = this->rectF().width()*((double)health/maxHealth);
    if(healthWidth < 1) healthWidth = 1;
    healthWidthTarget = healthWidth;

}

void Enemy::updateWidth(double amount)
{
    if(health < 1) {
        healthWidth = 1;
        healthWidthTarget = 1;
    } else {
        healthWidthTarget = this->rectF().width()*((double)health/maxHealth);
        if(healthWidthTarget < 1) healthWidthTarget = 1;
        if(amount > 0) healthWidth -= amount;
        if(healthWidth <= healthWidthTarget) {
            healthWidth = healthWidthTarget;
        }
    }
}

int Enemy::getDensity(int type)
{
    int density = 0;
    switch(type) {
    case ENEMY_FLIESE:
        density = 1;
        break;
    case ENEMY_GEBANNT:
        density = 1;
        break;
    case ENEMY_NEUSCHWUCHTEL:
        density = 2;
        break;
    case ENEMY_SCHWUCHTEL:
        density = 3;
        break;
    case ENEMY_SPENDER:
        density = 3;
        break;
    case ENEMY_ALTSCHWUCHTEL:
        density = 5;
        break;
    case ENEMY_MOD:
        density = 10;
        break;
    case ENEMY_ADMIN:
        density = 12;
        break;
    case ENEMY_LEGENDE:
        density = 20;
        break;
    }
    //density = 0;
    return density;
}

QRect Enemy::rect()
{
    //return QRect(pos.x(),pos.y(),pos.width(),pos.height());
    if(body==nullptr||!physicsInitialized) {
        return QRect(pos.x(),pos.y(),pos.width(),pos.height());
    }
    return QRect(Engine::numToNormal(body->GetPosition().x)-pos.width()/2,
                 Engine::numToNormal(body->GetPosition().y)-pos.height()/2,
                 pos.width(),
                 pos.height());
}

QRectF Enemy::rectF(int a)
{
    if(!physicsInitialized||body==nullptr) {
        if(!a) {
            return pos;
        } else {
            return QRectF(pos.x()+pos.width()/2,pos.y()+pos.height()/2,pos.width(),pos.height());
        }
    } else {
        if(!a) {
            return QRectF(Engine::numToNormal(body->GetPosition().x) - (pos.width()/2),
                          Engine::numToNormal(body->GetPosition().y) - (pos.height()/2),
                          pos.width(),
                          pos.height());
        } else {
            return QRectF(Engine::numToNormal(body->GetPosition().x),
                          Engine::numToNormal(body->GetPosition().y),
                          pos.width(),
                          pos.height());
        }
    }
}

QRectF Enemy::centerRect()
{
    pos = rectF();
    QRectF center = QRectF(pos.center().x() - 1, pos.center().y() - 1, 2, 2);
    return center;
}

QString Enemy::toString()
{
    pos = rectF();
    return QString::number(cpos) + "," +
            QString::number(dir) + "," +
            QString::number(health) + "," +
            QString::number(maxHealth) + "," +
            QString::number(maxStun) + "," +
            QString::number(ospeed) + "," +
            QString::number(pos.x()) + "," + QString::number(pos.y()) + "," + QString::number(pos.width()) + "," + QString::number(pos.height()) + "," +
            QString::number(preHealth) + "," +
            QString::number(speed) + "," +
            QString::number(stunned) + "," +
            QString::number(repost) + "," +
            QString::number(soonBanned) + "," +
            QString::number(type) + "," +
            QString::number(opacity) + "," +
            QString::number(path) + "," +
            QString::number(price) + "," +
            QString::number(animation) + "," +
            QString::number(poison) + "," +
            QString::number(maxPoison) + "," +
            QString::number(poisonDmg) + "," +
            QString::number(newdir) + "," +
            QString::number(blocked) + "," +
            QString::number(reload) + "," +
            QString::number(maxReload) + "," +
            QString::number(imgID);
}
