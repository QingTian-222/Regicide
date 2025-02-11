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
#include "animationcontroller.h"
#include "tanimation.h"
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
QPoint show_pile_loc={450,220};
QPoint show_pile_siz={141,201};
QPoint hand_loc={250,440};
QPoint hand_siz={101,141};
QPoint other_hand_siz={41,55};

QPoint hidden_Joker_loc={1010,40};
QPoint Joker_loc={860,40};

QPoint Joker_siz={91,91};

QRect draw_pile_rect={draw_pile_loc.x(),draw_pile_loc.y(),
                        draw_pile_siz.x(),draw_pile_siz.y()};
QRect show_pile_rect={show_pile_loc.x(),show_pile_loc.y(),
                        show_pile_siz.x(),show_pile_siz.y()};
QRect atk_pile_rect={140,90,show_pile_siz.x(),show_pile_siz.y()};
QRect disc_pile_rect={disc_pile_loc.x(),disc_pile_loc.y(),
                        disc_pile_siz.x(),disc_pile_siz.y()};
QRect boss_die_rect={60,-280,141,201};
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
int is_Ani=0;
bool sortway=0;
int seed=0;
int barMul=100;
int barWid=501;

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

ani* ma(QLabel* label,QRect ed,int duration){
    ani* an=new ani(label,"geometry");
    an->setStartValue(label->geometry());
    an->setEndValue(ed);
    an->setDuration(duration);
    an->setEasingCurve(QEasingCurve::OutCubic);
    return an;
}
ani* wait(int x) {
    ani* an=new ani(handbar[0],"geometry");
    an->setStartValue(handbar[0]->geometry());
    an->setEndValue(handbar[0]->geometry());
    an->setDuration(x);
    return an;
}
void barAnimation(QLabel* label,int aimVal){
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
    timer->start(30);

}
void moveAnimation(QLabel* label,QRect ed,int duration){
    ma(label,ed,duration)->start();
}
void moveAnimation(QLabel* label,QPoint end,QPoint endsize={-1,-1},int duration=400){//移动动画
    if(endsize.x()==-1) endsize=QPoint(label->width(),label->height());
    QRect ed=QRect(end.x(),end.y(),endsize.x(),endsize.y());
    moveAnimation(label,ed,duration);
}

void openCard(QLabel *label,int pictureId){//翻牌动画
    qDebug()<<"fp st";
    is_Ani++;
    double wid=label->width(),locx=label->x();
    double w=wid,x=locx;
    double aimw=0,aimx=x+wid/2;
    double step=4;
    QTimer* timer=new QTimer();
    QObject::connect(timer,&QTimer::timeout,timer,[=]() mutable{
        w+=(aimw-w)/step;x+=(aimx-x)/step;
        label->setGeometry((int)x,label->y(),(int)w,label->height());
        if(fabs(x-aimx)<1&&fabs(w-aimw)<1){
            label->setGeometry(aimx,label->y(),aimw,label->height());
            turnPic(label,pictureId);
            aimw=wid,aimx=locx;
            QTimer* timer2=new QTimer();
            QObject::connect(timer2,&QTimer::timeout,timer,[=]() mutable{
                w+=(aimw-w)/step;x+=(aimx-x)/step;
                label->setGeometry((int)x,label->y(),(int)w,label->height());
                if(fabs(x-aimx)<1&&fabs(w-aimw)<1){
                    label->setGeometry(aimx,label->y(),aimw,label->height());
                    is_Ani--;
                    qDebug()<<"fp ed";
                    timer2->stop();
                    timer2->deleteLater();
                    timer->deleteLater();
                }
            });
            timer2->start(12);
            timer->stop();
        }
    });
    timer->start(12);
}
int getPoint(int x){//获取点数
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
    qDebug()<<"deal start";
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
                        qDebug()<<"deal end";
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
    if(bossCard.size()==0){
        QMessageBox::information(this,"","Congratulation!");
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
        return;
    }
    openCard(bossCard.back().pic,bossCard.back().id);
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
        qDebug()<<"player_start";
        is_Ani--;
        ui->confirmButton->setVisible(true);
        ui->passButton->setVisible(true);
        ui->sortButton->setVisible(true);

        ui->Joker->setEnabled(true);
        moveAnimation(ui->Joker,Joker_loc,Joker_siz);
    }
}
void menu::modifyMyHandCard(){//动态调整手牌
    ui->passButton->setEnabled(false);
    is_Ani++;
    int y=440;
    QVector<int> startLoc={0,450,360,300,270,260,250,245,240};
    QVector<int> interval={0,0,180,140, 90,80,70,60,50};
    int n=handCard[me].size();
    ui->handCardBar->setValue(handCard[me].size());
    for(int i=0;i<n;i++){
        handCard[me][i].pic->raise();
        moveAnimation(handCard[me][i].pic,{startLoc[n]-90+i*(interval[n]+10),y},hand_siz,200);
    }
    QTimer::singleShot(200,this,[=](){
        is_Ani--;
        ui->passButton->setEnabled(true);
    });

}
void menu::modifyPlayerCard(int id){//动态调整其他玩家手牌
    QVector<int> startLoc={530,420,310};
    int x=40;
    int interval=20;
    std::vector<int> temp;
    for(int i=1;i<=player_num;i++){
        if(i!=me){
            temp.push_back(i);
            if(i==id){
                int s=temp.size()-1;
                handbar[s]->setValue(handCard[id].size());
                for(int j=0;j<(int)handCard[id].size();j++){
                    handCard[id][j].pic->raise();
                    moveAnimation(handCard[id][j].pic,{x+j*interval,startLoc[s]},other_hand_siz);
                }
                break;
            }
        }
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
        moveAnimation(showCard[i].pic,{start[n-1]+interval*i,y},show_pile_siz);
    }
}

QVector<QLabel*> namelabel,backlabel;

void menu::init(){//整体初始化
    ac=new AnimationController();
    seed=(unsigned)time(0);
    srand(seed);
    handbar.append(ui->handbar_2);
    handbar.append(ui->handbar_3);
    handbar.append(ui->handbar_4);
    namelabel.append(ui->name1);
    namelabel.append(ui->name2);
    namelabel.append(ui->name3);
    backlabel.append(ui->back1);
    backlabel.append(ui->back2);
    backlabel.append(ui->b);
    ui->confirmButton->setVisible(false);
    ui->passButton->setVisible(false);
    ui->sortButton->setVisible(false);
    ui->defendButton->setVisible(false);

    for(auto i:handbar) i->setMaximum(max_of_handcard[player_num]);
    for(int i=0;i<3;i++){
        if(i>=player_num-1){
            namelabel[i]->setVisible(false);
            backlabel[i]->setVisible(false);
            handbar[i]->setVisible(false);
        }
    }
    ui->handCardBar->setMaximum(max_of_handcard[player_num]);
    for(int i=1;i<=52;i++){
        QLabel *label=new QLabel(this);
        label->setScaledContents(true);
        label->setGeometry(show_pile_rect);
        label->setPixmap(QPixmap(":/image/image/Rigicide_00.png"));
        label->setVisible(true);
        label->raise();
        label->installEventFilter(this);
        map_of_activeLabel[label]=i;
        cards[i]=Card(label,i);
    }
    map_of_activeLabel[ui->Joker]=100;
    ui->Joker->installEventFilter(this);

}

void menu::setup(){//初始化游戏
    is_Ani=1;
    current_player=1;
    roundNum=1;
    sortway=0;
    gameCard.clear();
    deadCard.clear();
    ui->remainCard->setValue(0);
    ui->remainCard->setMaximum(52);
    for(int i=1;i<=4;i++) handCard[i].clear();
    bossCard.clear();
    //加入牌堆
    for(int i=1;i<=52;i++){
        cards[i].is_open=false;
        if(getPoint(i)>10) bossCard.push_back(cards[i]);
        else gameCard.push_back(cards[i]);
    }

    //洗牌
    std::sort(bossCard.begin(),bossCard.end(),[&](Card a,Card b){
        return getPoint(a.id)>getPoint(b.id);
    });
    shuffle(gameCard,0,gameCard.size()-1);
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
                    roundStart();
                    playerStart();
                });
            });
        }
        i++;
    });
    timer->start(20);


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
    ui->setupUi(this);
    init();
    setup();

}

void menu::click(int id){

    if(is_Ani) return;
    if(id==100){//使用Joker
        //屏幕震动,label消失,所有手牌依次翻面,全部进入弃牌堆,发牌
        is_Ani++;
        ui->Joker->setVisible(false);
        ui->confirmButton->setVisible(false);
        ui->passButton->setVisible(false);
        ui->sortButton->setVisible(false);

        int idx=0,mx=handCard[me].size();

        QTimer* timer=new QTimer;

        connect(timer,&QTimer::timeout,this,[=]() mutable{
            openCard(handCard[me][idx].pic,0);
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
            QTimer::singleShot(800,this,[=](){
                ui->confirmButton->setVisible(true);
                ui->passButton->setVisible(true);
                ui->sortButton->setVisible(true);
                is_Ani--;
            });
        });





        return;
    }
    for(int i=0;i<(int)showCard.size();i++){
        if(showCard[i].id==id){
            showCard.erase(showCard.begin()+i);
            int x=cards[id].pic->x(),y=hand_loc.y();
            moveAnimation(cards[id].pic,{x,y},hand_siz,200);
            return;
        }
    }
    for(int i=0;i<(int)handCard[me].size();i++){
        if(handCard[me][i].id==id){
            showCard.push_back(cards[id]);
            int x=cards[id].pic->x(),y=hand_loc.y()-40;
            moveAnimation(cards[id].pic,{x,y},hand_siz,200);
            break;
        }
    }

}
void menu::move(int id){
    if(is_Ani) return;
    if(id==100){//使用Joker
        QPoint pt=Joker_loc;
        pt.setY(pt.y()-10);
        moveAnimation(ui->Joker,pt,Joker_siz,100);
        return;
    }
    for(int i=0;i<(int)showCard.size();i++){
        if(showCard[i].id==id) return;
    }

    for(int i=0;i<(int)handCard[me].size();i++){
        if(handCard[me][i].id==id){
            int x=handCard[me][i].pic->x(),y=hand_loc.y()-20;
            moveAnimation(handCard[me][i].pic,{x,y},hand_siz,100);
            break;
        }
    }
}
void menu::leave(int id){
    if(is_Ani) return;
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

bool menu::eventFilter(QObject *obj, QEvent *event){
    int id=map_of_activeLabel[obj];//对象下标
    if (event->type() == QEvent::MouseButtonPress) {
        click(id);
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
        //结算花色
        if(suit!=bossSuit){
            //0-梅花，1-方片，2-红桃，3-黑桃
            auto sk=shake(cards[card_id].pic,50,20,5);
            ac->addAnimation(sk);
            qDebug()<<"shake";
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
                    moveAnimation(ui->att,rect_tp,200);
                    barAnimation(ui->att_label,bossAtk);

                });
                ac->addAnimation(wait(200));
            }
            ac->addAnimation(wait(300));

        }
    }else{
        //攻击
        bossHealth-=point;
        auto ai=ma(cards[card_id].pic,atk_pile_rect,150);
        ac->addAnimation(ai);
        int h=bossHealth;
        QObject::connect(ai,&QPropertyAnimation::finished,[=](){
            int heal[]={20,30,40};
            int x=getPoint(bossCard.back().id)-11;
            rect_tp=ui->health->geometry();
            rect_tp.setWidth((int)((1.0*h/heal[x]) *barWid));
            moveAnimation(ui->health,rect_tp,200);
            barAnimation(ui->heal_label,h);
        });
        ac->addAnimation(ma(cards[card_id].pic,cards[card_id].pic->geometry(),150));
        ac->addAnimation(wait(500));
    }




}

void menu::defend(){//收到伤害
    int sum=0;
    for(auto i:showCard){
        sum+=getAttack(i.id);
    }
    if(sum>=bossAtk){
        qDebug()<<"receive ATK";
        is_Ani++;
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
        QTimer::singleShot(500,[=](){

            qDebug()<<"receive ATK end";
            for(auto i:showCard){
                moveAnimation(i.pic,disc_pile_loc,disc_pile_siz);
            }
            modifyMyHandCard();//调整手牌
            ui->defendButton->setVisible(false);

            current_player=(current_player)%player_num+1;
            showCard.clear();
            playerStart();
        });

    }

}
void menu::defendMode(){

    ui->defendButton->setVisible(true);
    ui->sortButton->setVisible(true);
    ui->confirmButton->setVisible(false);
    ui->passButton->setVisible(true);

    ui->Joker->setEnabled(false);
    moveAnimation(ui->Joker,hidden_Joker_loc,Joker_siz);


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
        modifyMyHandCard();//调整手牌
        for(int i=0;i<player_num-1;i++)
            modifyPlayerCard(i);
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
                }
                QTimer::singleShot(500,this,[=](){
                    for(auto i:showCard){
                        deadCard.push_back(i);
                        moveAnimation(i.pic,disc_pile_loc,disc_pile_siz);
                        ui->deathCard->setValue(deadCard.size());
                    }
                    showCard.clear();
                    if(bossHealth==0){//荣誉击杀->boss加入gameCard
                        auto m=ma(bossCard.back().pic,draw_pile_rect,400);
                        connect(m,&QPropertyAnimation::finished,this,[=](){
                            current_player=(current_player)%player_num+1;
                            playerStart();
                            roundStart();
                        });

                        gameCard.push_back(bossCard.back());
                        openCard(bossCard.back().pic,0);
                        bossCard.back().is_open=0;
                        for(auto i:gameCard) i.pic->raise();
                        bossCard.pop_back();

                        QTimer::singleShot(500,[=](){
                            ui->remainCard->setValue(gameCard.size());
                            m->start();

                        });
                    }
                    else if(bossHealth<0){//击杀->boss刷新，轮到下家回合
                        auto m=ma(bossCard.back().pic,disc_pile_rect,400);
                        connect(m,&QPropertyAnimation::finished,this,[=](){
                            current_player=(current_player)%player_num+1;
                            playerStart();
                            roundStart();
                        });

                        deadCard.push_back(bossCard.back());
                        openCard(bossCard.back().pic,0);
                        bossCard.back().is_open=0;
                        bossCard.pop_back();
                        QTimer::singleShot(500,[=](){
                            ui->deathCard->setValue(deadCard.size());
                            m->start();
                        });
                    }else{//进入boss造成伤害环节
                        if(bossAtk>0){
                            is_Ani--;
                            defendMode();
                        }else{
                            current_player=(current_player)%player_num+1;
                            playerStart();
                        }
                    }
                });
                disconnect(ac,nullptr,nullptr,nullptr);
            });



        });

    });


}

void menu::on_confirmButton_clicked()
{
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
        attack();
        ui->confirmButton->setVisible(false);
        ui->passButton->setVisible(false);
        ui->sortButton->setVisible(false);
        ui->Joker->setEnabled(false);
        moveAnimation(ui->Joker,hidden_Joker_loc,Joker_siz);
        //is_Ani=1;
    }else{
        return;
    }

}



void menu::on_sortButton_clicked()
{

    showCard.clear();
    if(sortway==1){
        std::sort(handCard[me].begin(),handCard[me].end(),[&](Card a,Card b){
            return a.id<b.id;
        });
        modifyMyHandCard();
    }else{
        std::sort(handCard[me].begin(),handCard[me].end(),[&](Card a,Card b){
            if(getPoint(a.id)==getPoint(b.id)) return a.id<b.id;
            return getPoint(a.id)<getPoint(b.id);
        });
        modifyMyHandCard();
    }
    sortway^=1;
}


void menu::on_defendButton_clicked()
{
    defend();
}


void menu::on_passButton_clicked()
{

    if(bossAtk>0)
        defendMode();
    else{
        current_player=(current_player)%player_num+1;
        playerStart();
    }
}

