#include "menu.h"
#include "ui_menu.h"
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QMessageBox>
#include "QRandomGenerator"
#include "QDebug"
#include "QVector"
#include<deque>
#include <QPropertyAnimation>
#include<QFontDatabase>
#include "animationcontroller.h"
#include "tanimation.h"
#include <QTcpSocket>
typedef TAnimation ani;

int player_num=1;//游戏人数
int current_player=1;//当前操作的玩家
int roundNum=1;//总回合数
int me=1;

int max_of_handcard[]={0,8,7,6,5};
int num_of_joker[]={0,0,0,1,2};


QPoint draw_pile_loc={850,200};
QPoint draw_pile_siz={111,151};
QPoint boss_pile_loc={60,30};
QPoint boss_pile_siz={141,201};
QPoint disc_pile_loc={850,410};
QPoint disc_pile_siz={111,151};
QPoint show_pile_loc={450,200};
QPoint show_pile_siz={141,201};
QPoint hand_loc={250,440-10};
QPoint hand_siz={101,141};
QPoint other_hand_siz={41,61};

QPoint hidden_Joker_loc={860,-100};
QPoint Joker_loc={860,40};

QPoint Joker_siz={91,91};

QRect draw_pile_rect={draw_pile_loc.x(),draw_pile_loc.y(),
                        draw_pile_siz.x(),draw_pile_siz.y()};
QRect show_pile_rect={show_pile_loc.x(),show_pile_loc.y(),
                        show_pile_siz.x(),show_pile_siz.y()};
QRect atk_pile_rect={140,90,show_pile_siz.x(),show_pile_siz.y()};
QRect disc_pile_rect={disc_pile_loc.x(),disc_pile_loc.y(),
                        disc_pile_siz.x(),disc_pile_siz.y()};
QRect boss_atk_rect={350,330,141,201};
QRect init_rect={};
struct Card{
    QLabel *pic;
    int id;
    bool is_open=false;
    Card(QLabel* p,int I){pic=p,id=I;}
};

AnimationController* ac;
std::deque<Card> gameCard;//抽牌堆
std::deque<Card> deadCard;//弃牌堆
std::deque<Card> handCard[5];//手牌
std::deque<Card> bossCard;//敌人牌堆
std::deque<Card> cards(53,Card(nullptr,53));//1~52的所有牌
std::deque<Card> showCard;//打出牌堆
int bossHealth,bossAtk;

QVector<QProgressBar*> handbar;

QHash<QObject*,int> map_of_activeLabel;

QRect rect_tp;
QVector<int> idx(1010);
int is_Ani=0;//动画中
int seed=0;//随机种子
int barWid=501;

bool usejoker=0;//使用了Joker(单机模式)
int killcount=0;//击杀首领统计
int helpcount=0;//归化首领统计
int timecount=0;//总时间
bool haveStarted=0;//是否开始游戏
bool isMulti=0;//多人模式
QTcpSocket *socket;

QGroupBox* playerGroupBox[4];
QLabel* playerName[4];
QLabel* playerBg[4];
QLabel* pn[4];

bool bossWeak=0;

QString getQrc(int x){//获取资源文件路径
    QString num=("00"+QString::number(x)).right(2);
    return num;
}
void turnPic(QLabel *label,int x){//更改图片
    label->setPixmap(QPixmap(":/image/image/Rigicide_"+getQrc(x)+".png"));
}
ani* shake(QLabel* label,int duration=70, int range=6, int shakeCount=5){
    ani* an = new ani(label, "pos", label);
    QPoint sp = label->pos();
    an->setKeyValueAt(0.1,QPoint(sp.x() + range, sp.y()));
    an->setKeyValueAt(0.2,QPoint(sp.x() - range, sp.y()));
    an->setKeyValueAt(0.3,QPoint(sp.x(), sp.y() + range));
    an->setKeyValueAt(0.4,QPoint(sp.x(), sp.y() - range));
    an->setKeyValueAt(0.5, sp);
    an->setLoopCount(shakeCount);
    an->setEndValue(sp);
    an->setDuration(duration);
    return an;
}
QByteArray send(QString head,QString content){
    QString hd=head+content;
    QString len="000"+QString::number(hd.toUtf8().size()+3);
    hd=len.right(3)+hd;
    return hd.toUtf8();
}
ani* ma(QLabel* label,QRect ed,int duration,QEasingCurve curve=QEasingCurve::OutCubic){
    ani* an=new ani(label,"geometry");
    an->setStartValue(label->geometry());
    an->setEndValue(ed);
    an->setDuration(duration);
    an->setEasingCurve(curve);
    return an;
}
void button_ma(QPushButton* label,QPoint st,QPoint ed,int duration,int flag,QEasingCurve curve=QEasingCurve::OutCubic){
    QPoint aimSt={label->x(),label->y()};
    label->setEnabled(flag);
    label->setVisible(true);
    ani* an=new ani(label,"pos");
    an->setStartValue(aimSt);
    an->setEndValue(ed);
    an->setDuration(duration);
    an->setEasingCurve(curve);
    an->start();
}

void setOpacity(QLabel *label,double opacity){//设置透明度
    QGraphicsOpacityEffect* opa=new QGraphicsOpacityEffect(label);
    opa->setOpacity(opacity);
    label->setGraphicsEffect(opa);
}

ani* wait(int x) {
    ani* an=new ani(handbar[0],"geometry");
    an->setStartValue(handbar[0]->geometry());
    an->setEndValue(handbar[0]->geometry());
    an->setDuration(x);
    return an;
}
void barAnimation(QLabel* label,int aimVal){
    label->setVisible(true);
    QTimer *timer = new QTimer;
    QObject::connect(timer,&QTimer::timeout,[=](){
        int nowVal=label->text().toInt();
        if(nowVal>aimVal){
            nowVal--;
            label->setText(QString::number(nowVal));
        }else if(nowVal<aimVal){
            nowVal++;
            label->setText(QString::number(nowVal));
        }else{
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(20);

}
void moveAnimation(QLabel* label,QRect ed,int duration,QEasingCurve curve=QEasingCurve::OutCubic){
    ma(label,ed,duration,curve)->start();
}
void moveAnimation(QLabel* label,QPoint end,QPoint endsize={-1,-1},int duration=400,QEasingCurve curve=QEasingCurve::OutCubic){//移动动画
    if(endsize.x()==-1) endsize=QPoint(label->width(),label->height());
    QRect ed=QRect(end.x(),end.y(),endsize.x(),endsize.y());
    moveAnimation(label,ed,duration,curve);
}

void openCard(QLabel *label,int pictureId){//翻牌动画
    is_Ani++;
    int wid=label->width();
    int aimw=0,aimx=label->x()+wid/2;
    QRect startRect=label->geometry();
    QRect aimRect={aimx,label->y(),aimw,label->height()};
    auto an=ma(label,aimRect,200,QEasingCurve::InBack);
    an->start();
    QObject::connect(an,&QPropertyAnimation::finished,[=](){
        turnPic(label,pictureId);
        auto an2=ma(label,startRect,200,QEasingCurve::OutBack);
        an2->start();
        QObject::connect(an2,&QPropertyAnimation::finished,[=](){
            is_Ani--;
        });
    });
}
void menu::shakeWidget(int duration=50, int range=5, int shakeCount=5){
    QPropertyAnimation* animation = new QPropertyAnimation(this, "pos");
    QPoint sp = this->pos();
    animation->setKeyValueAt(0.1,QPoint(sp.x() + range, sp.y()));
    animation->setKeyValueAt(0.2,QPoint(sp.x() - range, sp.y()));
    animation->setKeyValueAt(0.3,QPoint(sp.x(), sp.y() + range));
    animation->setKeyValueAt(0.4,QPoint(sp.x(), sp.y() - range));
    animation->setKeyValueAt(0.5, sp);
    animation->setLoopCount(shakeCount);
    animation->setEndValue(sp);
    animation->setDuration(duration);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

}

void fadeLabel(QLabel* label,int duration,float start,float end){//透明度
    label->setVisible(true);
    QGraphicsOpacityEffect* opa=new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(opa);
    QPropertyAnimation* animation = new QPropertyAnimation(opa,"opacity",label);
    animation->setDuration(duration);
    animation->setStartValue(start);
    animation->setEndValue(end);
    if(end==0){
        QObject::connect(animation,&QPropertyAnimation::finished,[=](){
            label->setVisible(false);
        });
    }

    animation->start(QAbstractAnimation::DeleteWhenStopped);


}
void fadeLabel(QPushButton* label,int duration,float start,float end){//透明度
    label->setVisible(true);
    QGraphicsOpacityEffect* opa=new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(opa);
    QPropertyAnimation* animation = new QPropertyAnimation(opa,"opacity",label);
    animation->setDuration(duration);
    animation->setStartValue(start);
    animation->setEndValue(end);
    if(end==0){
        QObject::connect(animation,&QPropertyAnimation::finished,[=](){
            label->setVisible(false);
        });
    }

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

int getPoint(int x){//获取点数
    if(x==53||x==54) return 0;
    return (x-1)%13+1;
}
int getAttack(int x){
    x=getPoint(x);
    if(x<=10) return x;
    if(x==11) return 10;
    if(x==12) return 15;
    return 20;
}
int getSuit(int x){//获取花色,0-梅花，1-方片，2-红桃，3-黑桃
    return (x-1)/13;
}
int getran(int l,int r){
    return rand()%(r-l+1)+l;
}
void shuffle(std::deque<Card> &v,int l,int r){
    for(int i=l;i<=r;i++){
        std::swap(v[i],v[getran(l,r)]);
    }
}
bool all_full_card(){//是否全员满手牌
    for(int i=1;i<=player_num;i++)
        if((int)handCard[i].size()<max_of_handcard[player_num]) return false;
    return true;
}
QPoint getAimLoc(int player){//获取手牌的目标位置
    if(player==me){

        return {100,100};
    }else{

        return {200,200};
    }
}
QPoint getAimSiz(int player){
    if(player==me){
        return hand_siz;
    }else{
        return other_hand_siz;
    }
}
void menu::dealCard(int cardNum,int Player){//发牌
    QTimer* timer=new QTimer;
    int startPlayer=Player;
    is_Ani++;
    connect(timer,&QTimer::timeout,this,[=]() mutable{
        if(!all_full_card() && cardNum && gameCard.size()){
            while(1){

                if((int)handCard[startPlayer].size()<max_of_handcard[player_num]){
                    handCard[startPlayer].push_back(gameCard.back());
                    gameCard.pop_back();
                    ui->remainCard->setValue(gameCard.size());
                    if(startPlayer==me) modifyMyHandCard();
                    else modifyPlayerCard(startPlayer);
                    cardNum--;
                    startPlayer=startPlayer%player_num+1;
                    break;
                }else{
                    startPlayer=startPlayer%player_num+1;
                }

            }
        }else{
            QTimer::singleShot(300,this,[=](){
                QTimer* timer2=new QTimer;
                int i=0;
                connect(timer2,&QTimer::timeout,this,[=]() mutable{//翻开未翻开的手牌
                    if(i<(int)handCard[me].size()){
                        if(!handCard[me][i].is_open){
                            handCard[me][i].is_open=true;
                            openCard(handCard[me][i].pic,handCard[me][i].id);
                        }
                    }else{
                        is_Ani--;
                        timer2->stop();
                        timer2->deleteLater();
                        timer->deleteLater();
                    }
                    i++;
                });
                timer2->start(40);
            });
            timer->stop();
        }
    });
    timer->start(100);
}
void menu::roundStart(){//boss回合开始
    bossWeak=0;
    if(bossCard.size()==0){

        is_Ani++;

        for(int num=1;num<=4;num++)
            for(auto i:handCard[num]){
                if(i.is_open) openCard(i.pic,0);
                i.is_open=false;
            }
        for(auto i:showCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }
        for(auto i:bossCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }
        for(auto i:gameCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }


        QTimer::singleShot(1000,this,[=](){
            for(int i=1;i<=52;i++){
                moveAnimation(cards[i].pic,show_pile_rect,200);
            }
            QTimer::singleShot(300,this,[=](){
                is_Ani--;

                gameover(1);

            });
        });
        return;
    }
    openCard(bossCard.back().pic,bossCard.back().id);
    bossCard.back().is_open=1;
    int heal[]={20,30,40};
    int atk[]={10,15,20};
    int x=getPoint(bossCard.back().id)-11;
    bossHealth=heal[x];
    bossAtk=atk[x];
    ui->health->setGeometry(300,50,0,51);
    ui->att->setGeometry(300,110,0,31);
    barAnimation(ui->heal_label,bossHealth);

    rect_tp=ui->health->geometry();
    rect_tp.setWidth(barWid);
    moveAnimation(ui->health,rect_tp,600);
    rect_tp=ui->att->geometry();
    rect_tp.setWidth(barWid);
    moveAnimation(ui->att,rect_tp,600);
    barAnimation(ui->att_label,bossAtk);

}
void menu::playerStart(){//玩家回合开始
    if(me==current_player){
        //如果我没有手牌且敌方攻击力为0,判负
        if(!bossCard.size()) return;
        if(handCard[me].size()==0 && bossAtk==0 && usejoker){
            gameover(0);
            return;
        }
        //如果我没有手牌且没有Joker,判负
        if(handCard[me].size()==0 && usejoker){
            bossCard.back().pic->raise();
            QTimer::singleShot(400,this,[=](){
                auto ai=ma(bossCard.back().pic,boss_atk_rect,700,QEasingCurve::InBack);
                connect(ai,&QPropertyAnimation::finished,this,[=](){
                    shakeWidget();
                });
                ai->start();
                gameover(0);
            });

            return;
        }

        qDebug()<<bossAtk;
        if(bossAtk){
            button_ma(ui->passButton,{710,660},{710,530},600,1,QEasingCurve::OutBack);
        }
        button_ma(ui->confirmButton,{710,660},{710,470},600,1,QEasingCurve::OutBack);
        button_ma(ui->sortButton,{380,660},{380,587},800,1,QEasingCurve::OutBack);
        button_ma(ui->sortButton_2,{490,660},{490,587},800,1,QEasingCurve::OutBack);

        if(isMulti){

        }else{
            ui->Joker->setEnabled(true);
            moveAnimation(ui->Joker,Joker_loc,Joker_siz);
        }



    }
}
void menu::modifyMyHandCard(){//动态调整手牌
    ui->passButton->setEnabled(false);
    is_Ani++;
    int y=hand_loc.y();
    QVector<int> startLoc={0,450,360,300,270,260,250,245,240};
    QVector<int> interval={0,0,180,140, 90,80,70,60,50};
    int n=handCard[me].size();
    ui->handCardBar->setValue(handCard[me].size());

    for(int i=0;i<n;i++){
        handCard[me][i].pic->raise();
        moveAnimation(handCard[me][i].pic,{startLoc[n]-90+i*(interval[n]+10),y},hand_siz,250);
    }

    QTimer::singleShot(250,this,[=](){
        is_Ani--;
        ui->passButton->setEnabled(true);
    });

}
void menu::modifyPlayerCard(int id){//动态调整其他玩家手牌
    int locy[5];

    locy[(me+2)%player_num+1]=510;
    locy[(me+1)%player_num+1]=300;
    locy[(me)%player_num+1]=90;


    int inv[8]={0,0,-50,-46,-46,-33,-30,-24};
    int locx[8]={0,1104,1130,1150,1174,1176,1176,1178};


    int n=handCard[id].size();
    for(int j=0;j<n;j++){
        handCard[id][j].pic->raise();
        moveAnimation(handCard[id][j].pic,{locx[n]+inv[n]*j,locy[id]},other_hand_siz);
    }

}
void menu::modifyShowCard(){//动态调整展示出的手牌
    int x=show_pile_loc.x();
    int y=show_pile_loc.y();
    int n=showCard.size();
    int w=show_pile_rect.width();
    QVector<int> start={x,x-w/2,x-w,x-3*w/2};
    int interval=w+10;
    for(int i=0;i<n;i++){
        turnPic(showCard[i].pic,showCard[i].id);
        moveAnimation(showCard[i].pic,{start[n-1]+interval*i,y},show_pile_siz);
    }
}

QVector<QLabel*> namelabel,backlabel;

void menu::init(){//整体初始化
    ac=new AnimationController();
    handbar.append(ui->handCardBar);

    playerGroupBox[0]=ui->playergroup1;
    playerGroupBox[1]=ui->playergroup2;
    playerGroupBox[2]=ui->playergroup3;
    playerGroupBox[3]=ui->playergroup4;
    playerName[0]=ui->playername1;
    playerName[1]=ui->playername2;
    playerName[2]=ui->playername3;
    playerName[3]=ui->playername4;
    playerBg[0]=ui->playerbg1;
    playerBg[1]=ui->playerbg2;
    playerBg[2]=ui->playerbg3;
    playerBg[3]=ui->playerbg4;
    pn[1]=ui->pn1;
    pn[2]=ui->pn2;
    pn[3]=ui->pn3;

    ui->confirmButton->setVisible(false);
    ui->passButton->setVisible(false);
    ui->sortButton->setVisible(false);
    ui->sortButton_2->setVisible(false);
    ui->defendButton->setVisible(false);
    ui->gameoverBackground->setVisible(false);
    //ui->gameoverBackground_2->setVisible(false);
    ui->startOnline->setVisible(false);
    ui->killCount->setVisible(false);
    ui->helpCount->setVisible(false);
    ui->timeCount->setVisible(false);
    ui->retryButton->setVisible(false);
    ui->retryButton_2->setVisible(false);
    ui->gameoverTitle->setVisible(false);
    ui->goldenPrice->setVisible(false);
    ui->heal_label->setVisible(false);
    ui->att_label->setVisible(false);
    ui->multiplayer->setGeometry({0,240,0,231});
    ui->multiplayer_2->setGeometry({0,240,0,231});
    ui->bg->raise();
    ui->maintitle->raise();
    ui->gameoverBackground_2->raise();
    ui->aboutButton->raise();
    ui->exitButton->raise();
    ui->ruleButton->raise();
    ui->scoreButton->raise();

    for(auto i:handbar) i->setMaximum(max_of_handcard[player_num]);
//    for(int i=0;i<3;i++){
//        if(i>=player_num-1){
//            namelabel[i]->setVisible(false);
//            backlabel[i]->setVisible(false);
//            handbar[i]->setVisible(false);
//        }
//    }

    QRect fst={430,254,show_pile_siz.x(),show_pile_siz.y()};
    for(int i=1;i<=54;i++){
        QLabel *label=new QLabel(this);
        label->setScaledContents(true);
        label->setGeometry(fst);
        label->setPixmap(QPixmap(":/image/image/Rigicide_00.png"));
        label->setVisible(true);
        label->raise();
        label->installEventFilter(this);
        //setShadow(label);
        label->setCursor(Qt::PointingHandCursor);
        map_of_activeLabel[label]=i;
        cards[i]=Card(label,i);
    }
    map_of_activeLabel[ui->Joker]=100;
    ui->Joker->installEventFilter(this);
}

QTimer* mainTimer=new QTimer;
void menu::setup(){//初始化游戏
    is_Ani=1;
    current_player=1;
    roundNum=1;
    usejoker=0;
    killcount=0;
    helpcount=0;
    timecount=0;
    seed=(unsigned)time(0);
    srand(seed);
    connect(mainTimer,&QTimer::timeout,this,[=](){
        timecount++;
    });
    mainTimer->start(1000);

    gameCard.clear();
    deadCard.clear();
    ui->gameoverBackground->setVisible(false);
    ui->gameoverBackground_2->setVisible(false);
    ui->killCount->setVisible(false);
    ui->helpCount->setVisible(false);
    ui->timeCount->setVisible(false);
    ui->retryButton->setVisible(false);
    ui->retryButton_2->setVisible(false);
    ui->gameoverTitle->setVisible(false);
    ui->goldenPrice->setVisible(false);



    ui->gameoverBackground->raise();
    ui->gameoverBackground_2->raise();
    ui->gameoverTitle->raise();
    ui->helpCount->raise();
    ui->timeCount->raise();
    ui->killCount->raise();
    ui->goldenPrice->raise();
    ui->retryButton->raise();
    ui->retryButton_2->raise();

    ui->gameoverBackground_2->setGraphicsEffect(nullptr);
    ui->timeCount->setGraphicsEffect(nullptr);
    ui->helpCount->setGraphicsEffect(nullptr);
    ui->killCount->setGraphicsEffect(nullptr);

    ui->remainCard->setValue(0);
    ui->remainCard->setMaximum(52);
    ui->deathCard->setValue(0);
    ui->handCardBar->setValue(0);
    ui->Joker->setVisible(true);
    ui->Joker->setGraphicsEffect(NULL);
    ui->Joker->setGeometry(hidden_Joker_loc.x(),hidden_Joker_loc.y(),Joker_siz.x(),Joker_siz.y());
    for(int i=1;i<=4;i++) handCard[i].clear();
    bossCard.clear();
    //加入牌堆

    int total[5]={0,52,52,53,54};
    for(int i=1;i<=54;i++){

        cards[i].is_open=false;
        if(getPoint(i)>10) bossCard.push_back(cards[i]);
        else gameCard.push_back(cards[i]);
        if(i>total[player_num])
            cards[i].pic->setVisible(false);

    }




    //洗牌
    std::sort(bossCard.begin(),bossCard.end(),[&](Card a,Card b){
        return getPoint(a.id)>getPoint(b.id);
    });
    while(1){
        shuffle(gameCard,0,gameCard.size()-1);
        int sm=0;
        for(int i=gameCard.size()-1,j=0;j<8;i--,j++){
            if(getSuit(gameCard[i].id)==1) sm+=getAttack(gameCard[i].id);
        }
        if(sm>=8) break;
    }

    do{
        shuffle(bossCard,0,3);
        shuffle(bossCard,4,7);
        shuffle(bossCard,8,11);
    }while(getSuit(bossCard[3].id)==getSuit(bossCard[4].id) || getSuit(bossCard[7].id)==getSuit(bossCard[8].id)  );


    for(int i=0;i<(int)gameCard.size();i++){
        gameCard[i].pic->raise();
    }
    for(int i=0;i<(int)bossCard.size();i++){
        bossCard[i].pic->raise();
    }

    //动画
    QTimer* timer=new QTimer();
    int i=0;
    connect(timer,&QTimer::timeout,this,[=]() mutable{
        if(i<(int)gameCard.size()){
            moveAnimation(gameCard[i].pic,draw_pile_loc,draw_pile_siz);
            QTimer::singleShot(10,this,[=](){
                gameCard[i].pic->raise();
                ui->remainCard->setValue(ui->remainCard->value()+1);
            });
        }else{
            timer->stop();
            timer->deleteLater();
            QTimer::singleShot(300,this,[=](){
                dealCard(player_num*max_of_handcard[player_num],1);
                QTimer::singleShot(1500,this,[=](){
                    is_Ani--;
                    roundStart();

                    playerStart();
                });
            });
        }
        i++;
    });
    timer->start(30);


    QTimer* timer2=new QTimer();
    int j=0;
    connect(timer2,&QTimer::timeout,this,[=]() mutable{
        if(j<(int)bossCard.size()){
            moveAnimation(bossCard[j].pic,boss_pile_loc,boss_pile_siz);
            QTimer::singleShot(30,this,[=](){
                bossCard[j].pic->raise();
            });
        }else{
            timer2->stop();
            timer2->deleteLater();

        }
        j++;
    });
    timer2->start(60);
}

menu::menu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu)
{


    QFontDatabase::addApplicationFont(QCoreApplication::applicationDirPath()+"/font.ttf");
    QFontDatabase::addApplicationFont(QCoreApplication::applicationDirPath()+"/font2.ttf");
    ui->setupUi(this);

    init();
    //setup();

}

void menu::click(int id,int isLeft){

    if(is_Ani || current_player!=me) return;
    if(id==100 && isLeft && !usejoker){//使用Joker
        //屏幕震动,label消失,所有手牌依次翻面,全部进入弃牌堆,发牌
        is_Ani++;
        //ui->Joker->setVisible(false);
        usejoker=1;
        fadeLabel(ui->Joker,400,1,0);


        button_ma(ui->passButton,{710,530},{710,660},600,0,QEasingCurve::OutBack);
        button_ma(ui->confirmButton,{710,470},{710,660},600,0,QEasingCurve::OutBack);
        button_ma(ui->sortButton,{380,587},{380,660},800,0,QEasingCurve::OutBack);
        button_ma(ui->sortButton_2,{490,587},{490,660},800,0,QEasingCurve::OutBack);

        int idx=0,mx=handCard[me].size();

        QTimer* timer=new QTimer;

        connect(timer,&QTimer::timeout,this,[=]() mutable{
            openCard(handCard[me][idx].pic,0);
            handCard[me][idx].is_open=0;
            deadCard.push_back(handCard[me][idx]);
            int nw=deadCard.size();
            QTimer::singleShot(700,this,[=](){
                moveAnimation(handCard[me][idx].pic,disc_pile_loc,disc_pile_siz,300);
                ui->deathCard->setValue(nw);
            });
            idx++;
            if(idx==mx){
                timer->stop();
                timer->deleteLater();
            }
        });
        if(mx) timer->start(40);
        int waitTime=mx?1000+40*(mx-1):0;
        QTimer::singleShot(waitTime,this,[=](){
            handCard[me].clear();
            dealCard(8,1);
            QTimer::singleShot(1600,this,[=](){
                button_ma(ui->passButton,{710,660},{710,530},600,1,QEasingCurve::OutBack);
                button_ma(ui->confirmButton,{710,660},{710,470},600,1,QEasingCurve::OutBack);
                button_ma(ui->sortButton,{380,660},{380,587},800,1,QEasingCurve::OutBack);
                button_ma(ui->sortButton_2,{490,660},{490,587},800,1,QEasingCurve::OutBack);
                is_Ani--;
            });
        });
        return;
    }
    if(isLeft){//选中/取消选择
        for(int i=0;i<(int)showCard.size();i++){
            if(showCard[i].id==id){
                showCard.erase(showCard.begin()+i);
                int x=cards[id].pic->x(),y=hand_loc.y();
                moveAnimation(cards[id].pic,{x,y},hand_siz,200,QEasingCurve::OutBack);
                return;
            }
        }
        for(int i=0;i<(int)handCard[me].size();i++){
            if(handCard[me][i].id==id){
                showCard.push_back(cards[id]);
                int x=cards[id].pic->x(),y=hand_loc.y()-40;
                moveAnimation(cards[id].pic,{x,y},hand_siz,200,QEasingCurve::OutBack);
                break;
            }
        }
    }else{//取消全部
        showCard.clear();
        modifyMyHandCard();

    }


}
void menu::move(int id){
    if(is_Ani || current_player!=me) return;
    if(id==100){//使用Joker
        QPoint pt=Joker_loc;
        pt.setY(pt.y()-10);
        moveAnimation(ui->Joker,pt,Joker_siz,100,QEasingCurve::OutBack);
        return;
    }
    for(int i=0;i<(int)showCard.size();i++){
        if(showCard[i].id==id) return;
    }

    for(int i=0;i<(int)handCard[me].size();i++){
        if(handCard[me][i].id==id){
            int x=handCard[me][i].pic->x(),y=hand_loc.y()-20;
            moveAnimation(handCard[me][i].pic,{x,y},hand_siz,100,QEasingCurve::OutBack);
            break;
        }
    }
}
void menu::leave(int id){
    if(is_Ani || current_player!=me) return;
    if(id==100){//使用Joker
        QPoint pt=Joker_loc;
        moveAnimation(ui->Joker,pt,Joker_siz,100);
        return;
    }
    for(int i=0;i<(int)showCard.size();i++){
        if(showCard[i].id==id) return;
    }

    for(int i=0;i<(int)handCard[me].size();i++){
        if(handCard[me][i].id==id){
            int x=handCard[me][i].pic->x(),y=hand_loc.y();
            moveAnimation(handCard[me][i].pic,{x,y},hand_siz,200);
            break;
        }
    }

}

void menu::initAnimation(){
    fadeLabel(ui->bg,1000,1,0);
    fadeLabel(ui->maintitle,1000,1,0);
    fadeLabel(ui->gameoverBackground_2,1000,1,0);

    QRect tp=ui->gameoverBackground_2->geometry();
    tp.setWidth(0);
    moveAnimation(ui->gameoverBackground_2,tp,800);
    if(isMulti){
        ui->multiplayer->setVisible(false);
        ani* an=new ani(ui->multiplayer_2,"size",ui->multiplayer);
        an->setDuration(600);
        an->setEndValue(QSize(0,231));
        an->setEasingCurve(QEasingCurve::OutCubic);
        an->start();
        if(me==1) fadeLabel(ui->startOnline,200,1,0);
    }else{
        fadeLabel(ui->aboutButton,1000,1,0);
        fadeLabel(ui->ruleButton,1000,1,0);
        fadeLabel(ui->exitButton,1000,1,0);
        fadeLabel(ui->scoreButton,1000,1,0);

    }
    QTimer::singleShot(1500,this,[=](){
        if(isMulti){
            extendWidget(1251,641);
        }
       setup();
    });


}
bool menu::eventFilter(QObject *obj, QEvent *event){
    int id=map_of_activeLabel[obj];//对象下标
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() == Qt::LeftButton){
            if(haveStarted){
                click(id,1);
                qDebug()<<id;
            }else{
                haveStarted=1;
                initAnimation();
            }

        }else if (mouseEvent->buttons() == Qt::RightButton){
            click(id,0);
        }
    }else if (event->type() == QEvent::Enter) {
        move(id);
    } else if (event->type() == QEvent::Leave) {
        leave(id);
    }
    return QObject::eventFilter(obj, event);
}



menu::~menu()
{
    delete ui;
}
void menu::cure(int x){//治疗
    x=std::min(x,(int)deadCard.size());
    shuffle(deadCard,0,deadCard.size()-1);
    QTimer *timer=new QTimer();
    int i=0;
    connect(timer,&QTimer::timeout,this,[=]() mutable{
        if(i<x){
            gameCard.push_front(deadCard.back());
            for(auto i:gameCard) i.pic->raise();
            ui->remainCard->setValue(gameCard.size());
            moveAnimation(deadCard.back().pic,draw_pile_loc,draw_pile_siz);
            deadCard.pop_back();
            ui->deathCard->setValue(deadCard.size());

        }else{
            timer->stop();
            timer->deleteLater();
        }
        i++;
    });
    timer->start(80);

}
//单独卡牌计算
void menu::CaculateCard(int card_id,int val,int flag){

    int point=val;
    int suit=getSuit(card_id);
    int bossSuit=getSuit(bossCard.back().id);
    if(flag==0){
        if(getPoint(card_id)==0){
            bossWeak=1;
            return;
        }
        //结算花色
        if(suit!=bossSuit || bossWeak){
            //0-梅花，1-方片，2-红桃，3-黑桃
            auto sk=shake(cards[card_id].pic,50,20,5);
            ac->addAnimation(sk);
            if(suit==0){
                for(auto i:showCard){
                    CaculateCard(i.id,getAttack(i.id),1);
                }
            }else if(suit==1){
                connect(sk,&QPropertyAnimation::finished,this,[=](){
                    dealCard(point,current_player);
                });
                ac->addAnimation(wait(800));

            }else if(suit==2){
                if(deadCard.size()){
                    connect(sk,&QPropertyAnimation::finished,this,[=](){
                        cure(point);
                    });
                    ac->addAnimation(wait(80*point));
                }

            }else{
                bossAtk=std::max(0,bossAtk-point);

                connect(sk,&QPropertyAnimation::finished,this,[=](){

                    int atk[]={10,15,20};
                    int x=getPoint(bossCard.back().id)-11;
                    rect_tp=ui->att->geometry();
                    rect_tp.setWidth((int)((1.0*bossAtk/atk[x]) *barWid));
                    moveAnimation(ui->att,rect_tp,400,QEasingCurve::OutBack);
                    barAnimation(ui->att_label,bossAtk);

                });
                ac->addAnimation(wait(200));
            }
            ac->addAnimation(wait(200));

        }
    }else{


        bossHealth-=point;
        auto ai=ma(cards[card_id].pic,atk_pile_rect,200,QEasingCurve::InBack);
        connect(ai,&QPropertyAnimation::finished,this,[=](){
            if(point>=7) shakeWidget();
        });
        ac->addAnimation(ai);
        int h=bossHealth;
        connect(ai,&QPropertyAnimation::finished,this,[=](){
            int heal[]={20,30,40};
            int x=getPoint(bossCard.back().id)-11;
            rect_tp=ui->health->geometry();
            rect_tp.setWidth((int)((1.0*h/heal[x]) *barWid));
            moveAnimation(ui->health,rect_tp,200,QEasingCurve::OutBack);
            barAnimation(ui->heal_label,h);
        });
        ac->addAnimation(ma(cards[card_id].pic,cards[card_id].pic->geometry(),150));
        ac->addAnimation(wait(400));

    }



}
void menu::receive_attack(){
    bossCard.back().pic->raise();

    QRect aim;
    if(me==current_player){
        aim=boss_atk_rect;
    }else{
        QRect locy[5];
        locy[(me+2)%player_num+1]=ui->player3->geometry();
        locy[(me+1)%player_num+1]=ui->player2->geometry();
        locy[(me)%player_num+1]=ui->player1->geometry();
        aim=locy[current_player];
    }

    auto ai=ma(bossCard.back().pic,aim,400,QEasingCurve::InBack);
    connect(ai,&QPropertyAnimation::finished,this,[=](){
        if(bossAtk>=10) shakeWidget();
    });
    ai->start();
    connect(ai,&QPropertyAnimation::finished,this,[=](){
        moveAnimation(bossCard.back().pic,boss_pile_loc,boss_pile_siz,300);
    });
}

void menu::df(){
    is_Ani++;
    if(me==current_player){
        button_ma(ui->sortButton,{380,587},{380,660},800,0,QEasingCurve::OutBack);
        button_ma(ui->sortButton_2,{490,587},{490,660},800,0,QEasingCurve::OutBack);
    }else{
        for(auto i:showCard){
            openCard(i.pic,i.id);
            i.is_open=1;
        }
    }

    receive_attack();
    QTimer::singleShot(800,this,[=](){
        for(int j=0;j<(int)showCard.size();j++){
            for(int i=handCard[current_player].size()-1;i>=0;i--){//删除手牌
                if(handCard[current_player][i].id==showCard[j].id){
                    deadCard.push_back(showCard[j]);
                    ui->deathCard->setValue(deadCard.size());
                    handCard[current_player].erase(handCard[current_player].begin()+i);
                    break;
                }
            }
        }
        for(auto i:showCard){

            openCard(i.pic,0);
            i.is_open=0;
        }
        QTimer::singleShot(500,this,[=](){

            for(auto i:showCard){
                moveAnimation(i.pic,disc_pile_loc,disc_pile_siz);
            }

            if(me==current_player)modifyMyHandCard();//调整手牌
            else modifyPlayerCard(current_player);
            if(me==current_player) button_ma(ui->defendButton,{710,500},{710,660},600,0,QEasingCurve::OutBack);

            current_player=(current_player)%player_num+1;
            showCard.clear();
            is_Ani--;
            playerStart();
        });
    });
}

void menu::defend(){//收到伤害
    int sum=0;

    for(auto i:showCard){
        sum+=getAttack(i.id);
    }
    if(sum>=bossAtk){
        if(isMulti){
            QString res="";
            for(int i=0;i<(int)showCard.size();i++){
                res+=QString::number(showCard[i].id);
                if(i!=(int)showCard.size()-1) res+=',';
            }
            socket->write(send("DEF",res));
        }else{
            df();
        }



    }

}
void menu::defendMode(){

    if(isMulti){

    }else{
        ui->Joker->setEnabled(false);
        moveAnimation(ui->Joker,hidden_Joker_loc,Joker_siz);
    }


    int sum=0;
    for(auto i:handCard[me]) sum+=getAttack(i.id);
    if(bossAtk>sum){//游戏结束
        //gameoverBackground 淡出,title淡出,红色背景从左到右,统计项目从下至上,奖杯淡出,重试按钮淡出
        bossCard.back().pic->raise();
        auto ai=ma(bossCard.back().pic,boss_atk_rect,700,QEasingCurve::InBack);
        connect(ai,&QPropertyAnimation::finished,this,[=](){
            shakeWidget();
        });
        ai->start();
        gameover(0);
    }else{
        if(me==current_player){
            button_ma(ui->defendButton,{710,660},{710,500},600,1,QEasingCurve::OutBack);

            button_ma(ui->sortButton,{380,660},{380,587},800,1,QEasingCurve::OutBack);
            button_ma(ui->sortButton_2,{490,660},{490,587},800,1,QEasingCurve::OutBack);
        }

    }

}
void menu::gov(bool isWin){
    mainTimer->stop();
    button_ma(ui->sortButton,{380,587},{380,660},800,0,QEasingCurve::OutBack);
    button_ma(ui->sortButton_2,{490,587},{490,660},800,0,QEasingCurve::OutBack);
    button_ma(ui->passButton,{710,530},{710,660},600,0,QEasingCurve::OutBack);
    button_ma(ui->confirmButton,{710,470},{710,660},600,0,QEasingCurve::OutBack);

    if(isWin){
        ui->gameoverTitle->setText("游戏胜利");
        if(usejoker){
            ui->goldenPrice->setText("🏅");
        }else{
            ui->goldenPrice->setText("🏆");
        }
    }else{
        ui->gameoverTitle->setText("游戏结束");
        ui->goldenPrice->setText("🪦");
    }
    ui->killCount->setText("⚔️  "+QString::number(killcount)+"/12");
    ui->helpCount->setText("🤝  "+QString::number(helpcount)+"/12");
    int hours = timecount / 3600;
    int minutes = (timecount % 3600) / 60;
    int seconds = timecount % 60;
    QString timeString = QString("%1:%2:%3")
                            .arg(hours, 2, 10, QLatin1Char('0'))
                            .arg(minutes, 2, 10, QLatin1Char('0'))
                            .arg(seconds, 2, 10, QLatin1Char('0'));
    ui->timeCount->setText("⏱️ "+timeString);


    QTimer::singleShot(700,this,[=](){
        ui->gameoverBackground->raise();
        ui->gameoverBackground_2->raise();
        ui->gameoverTitle->raise();
        ui->helpCount->raise();
        ui->timeCount->raise();
        ui->killCount->raise();
        ui->goldenPrice->raise();
        ui->retryButton->raise();
        ui->retryButton_2->raise();
        fadeLabel(ui->gameoverBackground,500,0,1);
        QTimer::singleShot(150,this,[=](){
            fadeLabel(ui->gameoverTitle,500,0,1);
            QTimer::singleShot(250,this,[=](){

                QRect aimRect=ui->gameoverBackground_2->geometry();
                aimRect.setWidth(1001);
                QRect rt=aimRect;rt.setWidth(0);
                ui->gameoverBackground_2->setGeometry(rt);
                ui->gameoverBackground_2->setGraphicsEffect(NULL);
                ui->gameoverBackground_2->setVisible(true);
                moveAnimation(ui->gameoverBackground_2,aimRect,500);

                QRect waiting_rect={0,660,1001,71};
                QTimer::singleShot(550,this,[=](){
                    QRect kill_rect={0,250,1001,71};
                    ui->killCount->setGeometry(waiting_rect);
                    ui->killCount->setVisible(true);
                    moveAnimation(ui->killCount,kill_rect,600,QEasingCurve::OutBack);
                });
                QTimer::singleShot(950,this,[=](){
                    ui->helpCount->setGeometry(waiting_rect);
                    ui->helpCount->setVisible(true);
                    QRect help_rect={0,320,1001,71};
                    moveAnimation(ui->helpCount,help_rect,600,QEasingCurve::OutBack);
                });
                QTimer::singleShot(1350,this,[=](){
                    ui->timeCount->setGeometry(waiting_rect);
                    ui->timeCount->setVisible(true);
                    QRect time_rect={0,390,1001,71};
                    moveAnimation(ui->timeCount,time_rect,600,QEasingCurve::OutBack);
                });
                QTimer::singleShot(1950,this,[=](){
                    fadeLabel(ui->goldenPrice,400,0,1);
                    ui->goldenPrice->setVisible(true);
                    if(me==1){
                        if(isWin) ui->retryButton->setVisible(true);
                        else ui->retryButton_2->setVisible(true);
                    }

                });

            });
        });
    });
}

void menu::gameover(bool isWin){
    if(isMulti){
        int t=isWin;
        socket->write(send("GOV",QString::number(t)));
    }else{
        gov(isWin);
    }


}
void menu::attack(){
    //移动到展示区->造成伤害->抽牌->结算效果
    is_Ani++;
    modifyShowCard();//移动到展示区
    QTimer::singleShot(400,this,[=](){//等待
        for(int j=0;j<(int)showCard.size();j++){
            for(int i=handCard[current_player].size()-1;i>=0;i--){//删除手牌
                if(handCard[current_player][i].id==showCard[j].id){
                    handCard[current_player].erase(handCard[current_player].begin()+i);
                    break;
                }
            }
        }

        if(me==current_player) modifyMyHandCard();//调整手牌
        else modifyPlayerCard(current_player);


        QTimer::singleShot(400,this,[=](){//伤害动画
            int sum=0;
            for(auto i:showCard) sum+=getAttack(i.id);
            ac->clear();


            for(int i=0;i<(int)showCard.size();i++){
                if(i==0 || (getSuit(showCard[i].id)!=getSuit(showCard[i-1].id)))
                    CaculateCard(showCard[i].id,sum,0);
            }

            for(auto i:showCard){
                CaculateCard(i.id,getAttack(i.id),1);
                i.is_open=false;
            }
            //播放动画
            ac->startAnimations();

            connect(ac,&AnimationController::finished,this,[=](){
                for (auto i:showCard) {
                    openCard(i.pic, 0);
                    i.is_open=0;
                }
                QTimer::singleShot(500,this,[=](){
                    for(auto i:showCard){
                        deadCard.push_back(i);
                        moveAnimation(i.pic,disc_pile_loc,disc_pile_siz);
                        ui->deathCard->setValue(deadCard.size());
                    }
                    showCard.clear();
                    if(bossHealth==0){//荣誉击杀->boss加入gameCard
                        killcount++;
                        helpcount++;
                        auto m=ma(bossCard.back().pic,draw_pile_rect,400);
                        connect(m,&QPropertyAnimation::finished,this,[=](){
                            current_player=(current_player)%player_num+1;
                            is_Ani--;
                            roundStart();
                            playerStart();

                        });


                        openCard(bossCard.back().pic,0);
                        bossCard.back().is_open=0;
                        gameCard.push_back(bossCard.back());
                        for(auto i:gameCard) i.pic->raise();
                        bossCard.pop_back();

                        QTimer::singleShot(500,this,[=](){
                            ui->remainCard->setValue(gameCard.size());
                            m->start();

                        });
                    }
                    else if(bossHealth<0){//击杀->boss刷新，轮到下家回合
                        killcount++;
                        auto m=ma(bossCard.back().pic,disc_pile_rect,400);
                        connect(m,&QPropertyAnimation::finished,this,[=](){
                            current_player=(current_player)%player_num+1;
                            is_Ani--;
                            roundStart();
                            playerStart();

                        });


                        openCard(bossCard.back().pic,0);
                        bossCard.back().is_open=0;
                        deadCard.push_back(bossCard.back());
                        bossCard.pop_back();
                        QTimer::singleShot(500,this,[=](){
                            ui->deathCard->setValue(deadCard.size());
                            m->start();
                        });
                    }else{//进入boss造成伤害环节
                        if(bossAtk>0){
                            is_Ani--;
                            defendMode();
                        }else{
                            current_player=(current_player)%player_num+1;
                            is_Ani--;
                            playerStart();
                        }
                    }
                });
                disconnect(ac,nullptr,nullptr,nullptr);
            });



        });

    });


}

void menu::confirm(){
    int n=showCard.size();
    if(!n) return;
    bool all_the_same=1,have1=0;
    int sum=0;
    for(int i=0;i<n;i++){
        int point=getPoint(showCard[i].id);
        sum+=point;
        if(point==1) have1=1;
        if(point!=getPoint(showCard[0].id))
            all_the_same=0;
    }

    if(n==1||(n>1 && all_the_same && sum<=10)||(n==2 && have1)){//普通攻击
        if(isMulti){
            QString res="";
            for(int i=0;i<(int)showCard.size();i++){
                res+=QString::number(showCard[i].id);
                if(i!=(int)showCard.size()-1) res+=',';
            }
            socket->write(send("SHW",res));
        }else{
            attack();
            button_ma(ui->passButton,{710,530},{710,660},600,0,QEasingCurve::OutBack);
            button_ma(ui->confirmButton,{710,470},{710,660},600,0,QEasingCurve::OutBack);

            button_ma(ui->sortButton,{380,587},{380,660},800,0,QEasingCurve::OutBack);
            button_ma(ui->sortButton_2,{490,587},{490,660},800,0,QEasingCurve::OutBack);

            if(!isMulti){
                ui->Joker->setEnabled(false);
                moveAnimation(ui->Joker,hidden_Joker_loc,Joker_siz);
            }
        }





    }else{
        return;
    }
}

void menu::on_confirmButton_clicked()
{
    confirm();
}


void menu::on_sortButton_clicked()
{
    showCard.clear();
    std::sort(handCard[me].begin(),handCard[me].end(),[&](Card a,Card b){
        return a.id<b.id;
    });

    modifyMyHandCard();
}
void menu::on_sortButton_2_clicked()
{
    showCard.clear();
    std::sort(handCard[me].begin(),handCard[me].end(),[&](Card a,Card b){
        if(getPoint(a.id)==getPoint(b.id)) return a.id<b.id;
        return getPoint(a.id)<getPoint(b.id);
    });
    modifyMyHandCard();
}


void menu::on_defendButton_clicked()
{
    defend();
}


void menu::on_passButton_clicked()
{
    if(is_Ani) return;
    if(bossAtk>0){
        button_ma(ui->passButton,{710,530},{710,660},600,0,QEasingCurve::OutBack);
        button_ma(ui->confirmButton,{710,470},{710,660},600,0,QEasingCurve::OutBack);

        defendMode();
    }else{
        current_player=(current_player)%player_num+1;
        playerStart();
    }
}




void menu::on_retryButton_clicked()
{
    if(isMulti){
        socket->write(send("RTA",""));
    }else{
        fadeLabel(ui->gameoverBackground,400,1,0);
        fadeLabel(ui->gameoverBackground_2,400,1,0);
        fadeLabel(ui->gameoverTitle,400,1,0);
        fadeLabel(ui->helpCount,400,1,0);
        fadeLabel(ui->killCount,400,1,0);
        fadeLabel(ui->timeCount,400,1,0);
        fadeLabel(ui->goldenPrice,400,1,0);
        ui->retryButton->setVisible(false);
        QTimer::singleShot(500,this,[=](){
            setup();
        });
    }

}
void menu::RTB(){
    fadeLabel(ui->gameoverBackground,400,1,0);
    fadeLabel(ui->gameoverBackground_2,400,1,0);
    fadeLabel(ui->gameoverTitle,400,1,0);
    fadeLabel(ui->helpCount,400,1,0);
    fadeLabel(ui->killCount,400,1,0);
    fadeLabel(ui->timeCount,400,1,0);
    fadeLabel(ui->goldenPrice,400,1,0);
    ui->retryButton_2->setVisible(false);
    QTimer::singleShot(500,this,[=](){
        is_Ani++;
        for(int num=1;num<=4;num++)
            for(auto i:handCard[num]){
                if(i.is_open) openCard(i.pic,0);
                i.is_open=false;
            }
        for(auto i:showCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }
        for(auto i:bossCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }
        for(auto i:gameCard){
            if(i.is_open) openCard(i.pic,0);
            i.is_open=false;
        }
        QTimer::singleShot(1000,this,[=](){
            for(int i=1;i<=52;i++){
                moveAnimation(cards[i].pic,show_pile_rect,200);
            }
            QTimer::singleShot(300,this,[=](){
                is_Ani--;
                setup();
            });
        });
    });
}

void menu::on_retryButton_2_clicked()
{
    if(isMulti){
        socket->write(send("RTB",""));
    }else RTB();
}


void menu::on_aboutButton_clicked()
{
    QMessageBox::information(this,"开发者","游戏名:REGICIDE弑君者\n"
                                             "程序化:青天零云\n"
                                                "当前版本完成时间:2025/2/15");
}


void menu::on_exitButton_clicked()
{
    QApplication::quit();
}


void menu::on_ruleButton_clicked()
{
    //extendWidget(1251,641);
}
void menu::extendWidget(int w,int h){
    ani* an=new ani(this,"size");
    an->setEndValue(QSize(w,h));
    an->setEasingCurve(QEasingCurve::OutBounce);
    an->setDuration(1000);
    an->start();

}


void menu::on_scoreButton_clicked()
{
    isMulti^=1;
    if(isMulti){
        ui->scoreButton->setText("单人");
        ui->multiplayer->raise();
        ani* an=new ani(ui->multiplayer,"size",ui->multiplayer);
        an->setDuration(600);
        an->setEndValue(QSize(1001,231));
        an->setEasingCurve(QEasingCurve::OutCubic);
        an->start();
    }else{
        ui->scoreButton->setText("多人");
        ani* an=new ani(ui->multiplayer,"size",ui->multiplayer);
        an->setDuration(800);
        an->setEndValue(QSize(0,231));
        an->setEasingCurve(QEasingCurve::OutCubic);
        an->start();
    }


}

QByteArray buffer;
void menu::connect_server(QString name,QString ip,QString room)
{
    socket = new QTcpSocket(this);
    int port=8080;
    socket->connectToHost(ip,port);
    connect(socket,&QTcpSocket::connected,this,[=](){//连接成功
        ui->roomTitle->setText("room: "+ui->room->text());
        //

        socket->write(send("HEL",name+","+room));//发送身份验证

        //动画
        ui->multiplayer_2->raise();
        ani* an=new ani(ui->multiplayer_2,"size",ui->multiplayer_2);
        an->setDuration(600);
        an->setEndValue(QSize(1001,231));
        an->setEasingCurve(QEasingCurve::OutCubic);
        an->start();

        for(int i=0;i<4;i++){
            playerGroupBox[i]->raise();
            playerGroupBox[i]->setVisible(true);
            playerBg[i]->setVisible(false);
            playerName[i]->setVisible(false);

        }
        fadeLabel(ui->aboutButton,400,1,0);
        fadeLabel(ui->ruleButton,400,1,0);
        fadeLabel(ui->exitButton,400,1,0);
        fadeLabel(ui->scoreButton,400,1,0);

    });
    connect(socket,&QTcpSocket::readyRead,this,[=](){
        buffer.append(socket->readAll());
        qDebug()<<"!!!"<<buffer;
        while(1){
            if(buffer.size()<3) return;
            bool ok=false;
            int len=buffer.mid(0,3).toInt(&ok);
            if(!ok) return;
            if(buffer.size()<len) return;
            QString category=buffer.mid(3,3);
            QString data=buffer.mid(6,len-6);
            buffer=buffer.mid(len);

            qDebug()<<category;
            qDebug()<<data;
            if(category=="ROM"){//房间信息
                if(haveStarted) return;
                QStringList strs=data.split(",");
                player_num=strs.size();
                for(int i=0;i<4;i++){
                    playerGroupBox[i]->raise();
                    playerGroupBox[i]->setVisible(true);

                    if(i<strs.size()){
                        if(strs[i]==name){
                            me=i+1;
                            qDebug()<<"me"<<me;
                            if(me==1){//房主
                                ui->startOnline->raise();
                                fadeLabel(ui->startOnline,500,0,1);
                            }
                        }
                        playerBg[i]->setVisible(true);
                        playerName[i]->setVisible(true);
                        playerName[i]->setText(strs[i]);
                    }else{
                        playerBg[i]->setVisible(false);
                        playerName[i]->setVisible(false);
                    }


                }
                int locy[4];
                locy[(me+2)%player_num+1]=2;
                locy[(me+1)%player_num+1]=1;
                locy[me%player_num+1]=0;
                for(int i=1;i<=player_num;i++){
                    if(i!=me){
                        pn[locy[i]+1]->setText(strs[i-1]);
                    }
                }
            }else if(category=="SED"){
                seed=data.toInt();
                qDebug()<<seed;
            }else if(category=="STG"){//开始游戏
                haveStarted=1;
                ui->handCardBar->setMaximum(max_of_handcard[player_num]);

                initAnimation();
            }else if(category=="SHW"){//出牌
                if(me==current_player){
                    attack();
                    button_ma(ui->passButton,{710,530},{710,660},600,0,QEasingCurve::OutBack);
                    button_ma(ui->confirmButton,{710,470},{710,660},600,0,QEasingCurve::OutBack);

                    button_ma(ui->sortButton,{380,587},{380,660},800,0,QEasingCurve::OutBack);
                    button_ma(ui->sortButton_2,{490,587},{490,660},800,0,QEasingCurve::OutBack);

                }else{
                    QStringList strs=data.split(",");
                    for(int i=0;i<strs.size();i++){
                        for(int j=0;j<handCard[current_player].size();j++){
                            if(handCard[current_player][j].id==strs[i].toInt()){
                                showCard.push_back(handCard[current_player][j]);

                            }
                        }
                    }
                    modifyPlayerCard(current_player);
                    attack();
                }
            }else if(category=="DEF"){//出牌
                if(me==current_player){
                    df();

                }else{
                    QStringList strs=data.split(",");
                    for(int i=0;i<strs.size();i++){
                        for(int j=0;j<handCard[current_player].size();j++){
                            if(handCard[current_player][j].id==strs[i].toInt()){
                                showCard.push_back(handCard[current_player][j]);

                            }
                        }
                    }
                    modifyPlayerCard(current_player);
                    df();
                }
            }else if(category=="GOV"){
                gov(data.toInt());
            }else if(category=="RTA"){
                fadeLabel(ui->gameoverBackground,400,1,0);
                fadeLabel(ui->gameoverBackground_2,400,1,0);
                fadeLabel(ui->gameoverTitle,400,1,0);
                fadeLabel(ui->helpCount,400,1,0);
                fadeLabel(ui->killCount,400,1,0);
                fadeLabel(ui->timeCount,400,1,0);
                fadeLabel(ui->goldenPrice,400,1,0);
                ui->retryButton->setVisible(false);
                QTimer::singleShot(500,this,[=](){
                    setup();
                });
            }else if(category=="RTB"){
                RTB();
            }

        }
    });

}
void menu::on_connect_clicked()//连接到服务器
{
    connect_server(ui->name->text(),ui->ip->text(),ui->room->text());
    ui->connect->setEnabled(false);
    ui->connect->setText("connecting...");
}


void menu::on_startOnline_clicked()
{
    socket->write(send("RDY",""));//准备完成
}

