// 按鈕需要接 10k 歐姆的電阻
// 這個DS3231函式庫有一個bug，當每個時間要進位變成個位數時，後面會多一個數字 (之後有加程式碼debug) 58 -> 59 -> 19 -> 29
// 這組繼電器是低電頻觸發，就是輸出LOW電壓給他，才會觸發所連接的電器
//15s
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiNINA.h>
// Init the DS3231 using the hardware interface
DS3231  Clock;
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool Century = false;
bool h12, PM;
int moistSensor = A0;
int watering = 13;
int NoWaterWarning = 3;

int moist; //濕度
int waterAmount = 3; //澆水秒數
int changeMode = 0; //選擇澆水或設定澆水 (變換畫面用)
int option = 0; //左右的選項
int hour2 = 0, minute2 = 0;
int arrow = 0; //setTime的左右選項 
int moisTempChange = 0;

int today = 1; //設定避免同一個時間 一直重複澆水 
int detect = 0; //檢測未澆水的次數
int rr = 0;

/////////連線部分//////////

int Link = WL_IDLE_STATUS;
char ssid[]="Pineapple";
char pass[]="0988436306";
int a = 1;

////////雲端部分//////////

char server[]= "api.thingspeak.com";
WiFiClient client;
WiFiClient client2;
int val, val2, temp = 0;
String upload, upload2;

/////////////////////////

void Clock_default()// 初始時間函式
{
  Clock.setSecond(0); 
  Clock.setMinute(56);
  Clock.setHour(8);
  //Clock.setDoW(1);
  Clock.setDate(11);
  Clock.setMonth(6);
  Clock.setYear(19);
  
}
void ReadDS3231()
{
  int second,minute,hour,date,month,year,temperature;
  second=Clock.getSecond();
  minute=Clock.getMinute();
  hour=Clock.getHour(h12, PM);
  date=Clock.getDate();
  month=Clock.getMonth(Century);
  year=Clock.getYear();
  temperature=Clock.getTemperature();

  if(changeMode == 0 || changeMode == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("Time:");

    if(moisTempChange == 0) //顯示溫度
    {
      lcd.setCursor(16, 0);
      lcd.print(temperature);
      lcd.print("'C");
    }else if(moisTempChange == 1)   //顯示濕度
    { 
      if(moist < 100)
      {
        lcd.setCursor(17, 0);
        lcd.print(" ");
      }
      lcd.setCursor(15, 0);
      lcd.print(moist);
      lcd.setCursor(19, 0);
      lcd.print("M");
      delay(500);
    }
    
  
    lcd.setCursor(0, 1);
    lcd.print("20"); 
    lcd.print(year,DEC);
    lcd.print('-');
    lcd.print(month,DEC);
    lcd.print('-');
    lcd.print(date,DEC);
    
    if(hour == 0)
    {
      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.setCursor(11, 1);
      lcd.print(hour,DEC);
    }else{
      lcd.setCursor(11, 1);
      lcd.print(hour,DEC);
    }
    
    lcd.setCursor(13, 1);
    lcd.print(':');

    if(minute == 0)
    {
      lcd.setCursor(15, 1);
      lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.print(minute,DEC);
    }else{
      lcd.setCursor(14, 1);
      lcd.print(minute,DEC);
    }
    
    lcd.setCursor(16, 1);
    lcd.print(':');

    if(second == 0)
    {
      lcd.setCursor(18, 1);
      lcd.print(" ");
      lcd.setCursor(17, 1);
      lcd.print(second,DEC);
    }else{
      lcd.setCursor(17, 1);
      lcd.print(second,DEC);
    }  
  }
  
  
  
  //delay(1000); //每一秒刷新

}
void Functional()
{
    if(changeMode == 0)
    {
      lcd.setCursor(3, 3);
      lcd.print("Water");
      lcd.setCursor(11, 3);
      lcd.print("Setting");
    }
    
    else if(changeMode == 1)  //按下water
    {
      int dot = 0;
      lcd.clear();
      lcd.setCursor(4, 2);
      lcd.print("Watering ");
      digitalWrite(watering, LOW);  //開啟馬達
      for(int i=0;i <waterAmount ;i++)   //設定澆水量
      {
        if(dot == 3) // 清除那個 watering... 的小點點而已
        { 
          lcd.setCursor(13, 2);
          lcd.print("   ");
          lcd.setCursor(13, 2);
          dot = 0;
        }
        dot++;
        lcd.print(".");
        delay(1000);        //設定澆水澆多久 總共(waterAmount)秒
      }         
      digitalWrite(watering, HIGH);
      lcd.clear();
      changeMode = 0;
      
    }
    else if(changeMode == 2) //按下setting
    {
      //lcd.setCursor(3, 3);
      //lcd.print("               ");
      
      lcd.setCursor(3, 3);
      lcd.print("Amount");
      lcd.setCursor(11, 3);
      lcd.print("TimeSet");
      
    }
    else if(changeMode == 3) //按下amount
    {
      lcd.setCursor(3, 2);
      lcd.print("WaterAmount = ");
      lcd.setCursor(17, 2);
      lcd.print(waterAmount);
    }
    else if(changeMode == 4) //按下timeset
    {
      
      /*hour2 = Clock.getHour(h12, PM);
      minute2 = Clock.getMinute(); */

      lcd.setCursor(3, 2);
      lcd.print("TimeSet: ");
      
      lcd.setCursor(12, 2);
      lcd.print(hour2);
      lcd.setCursor(14, 2);
      lcd.print(":");
      lcd.setCursor(15, 2);
      lcd.print(minute2);
     

    }
    //delay(1000);
}
void Input()
{
 
  //int confirm = digitalRead(5);
  
  if(digitalRead(5) == HIGH) //按下確定
  {
    if(changeMode == 0 && option == 1)
    {
      changeMode = 1;
      option = 0;
      delay(300);
    }
    else if(changeMode == 0 && option == 2)
    {
      changeMode = 2;
      option = 0;
      lcd.clear();
      delay(300);
    }
    else if(changeMode == 2 && option == 1)
    {
      changeMode = 3;
      option = 0;
      lcd.clear();
      delay(300);
    }
    else if(changeMode == 2 && option == 2)
    {
      hour2 = Clock.getHour(h12, PM);
      minute2 = Clock.getMinute();
      changeMode = 4;
      option = 0;
      lcd.clear();
      delay(300);
    }
    else if(changeMode == 3) //設定澆水量
    {
      lcd.setCursor(1, 2);
      lcd.print("Setting Successful!");
      delay(2000);
      lcd.clear();
      changeMode = 0;
      delay(300);
    }
    else if(changeMode == 4) //設定時間
    {

      today = 0;
      lcd.setCursor(12, 1);
      lcd.print(" ");
      lcd.setCursor(15, 1);
      lcd.print(" ");
      lcd.setCursor(1, 2);
      lcd.print("Setting Successful!");
      delay(2000);
      lcd.clear();
      changeMode = 0;
      
      delay(300);
    }   
  }
  /////////////////////////////////
  
  if(digitalRead(6) == HIGH) //按下切換
  {
    if(changeMode == 0)
    {
      if(option == 2 || option == 0)
      {
        lcd.setCursor(2, 3);
        lcd.print("~");
        lcd.setCursor(10, 3);
        lcd.print(" ");
        option = 1;
        delay(500);
      }
      else if(option == 1)
      {
        lcd.setCursor(10, 3);
        lcd.print("~");
        lcd.setCursor(2, 3);
        lcd.print(" ");
        option = 2;
        delay(500);
      }
    }
    else if(changeMode == 2)
    {
      if(option == 2 || option == 0)
      {
        lcd.setCursor(2, 3);
        lcd.print("~");
        lcd.setCursor(10, 3);
        lcd.print(" ");
        option = 1;
        delay(500);
      }
      else if(option == 1)
      {
        lcd.setCursor(10, 3);
        lcd.print("~");
        lcd.setCursor(2, 3);
        lcd.print(" ");
        option = 2;
        delay(500);
      }
        
    }
    else if(changeMode == 4)
    {
      if(arrow == 2 || arrow == 0)
      {
        lcd.setCursor(12, 1);
        lcd.print("V");
        lcd.setCursor(15, 1);
        lcd.print(" ");
        arrow = 1;
        delay(500);
      }
      else if(arrow == 1)
      {
        lcd.setCursor(15, 1);
        lcd.print("V");
        lcd.setCursor(12, 1);
        lcd.print(" ");
        arrow = 2;
        delay(500);
      }
    }
  }
  /////////////////////////////
  if(digitalRead(7) == HIGH) //按上
  {
    if(changeMode == 0 || changeMode == 2)  //切換溫度顯示 跟 濕度顯示
    {
      if(moisTempChange == 0)
      {
        lcd.setCursor(15, 0);
        lcd.print("     ");
        moisTempChange = 1;
        delay(200);
      }else if(moisTempChange == 1)
      {
        lcd.setCursor(15, 0);
        lcd.print("    ");
        moisTempChange = 0;
        delay(200);
      }
    }
    else if(changeMode == 3)
    {
      waterAmount = waterAmount + 1;
      delay(300);
    }
    else if(changeMode == 4)
    {
      if(arrow == 1)
      {
        if(hour2 == 24)
        {
          lcd.setCursor(13, 2);
          lcd.print(" ");
          hour2 = 1;       
        }
        else{
          
          hour2 = hour2 + 1;
        }
        
        delay(300);
      }
      else if(arrow == 2)
      {
        if(minute2 == 60)
        {
          lcd.setCursor(16, 2);
          lcd.print(" ");
          minute2 = 0; 
        }
        else{

            minute2 = minute2 + 1;
          
        }
        
        delay(300);
      }
    }
  }
  ////////////////////////////////
  if(digitalRead(8) == HIGH) //按下
  {
    if(changeMode == 3)
    {
      waterAmount = waterAmount - 1;
      delay(300);
    }
    else if(changeMode == 4)
    {
      if(arrow == 1)
      {
        if(hour2 == 1)
        {
          hour2 = 24; 
        }
        else{
          if(hour2 == 10)
          {      
            hour2 = hour2 - 1;
            lcd.setCursor(13, 2);
            lcd.print(" ");
          }
          else{
            hour2 = hour2 - 1;
          }
          
        }
        
        delay(300);
      }
      else if(arrow == 2)
      {
        if(minute2 == 0)
        {
          minute2 = 59; 
        }
        else{
          if(minute2 == 10)
          {      
            minute2 = minute2 - 1;
            lcd.setCursor(16, 2);
            lcd.print(" ");
          }
          else{
            minute2 = minute2 - 1;
          }
          
        }
        
        delay(300);
      }
    }
  }
}

void AutoWatering() //要記得設定一天澆的次數 (done)
{
  moist = analogRead(moistSensor);
  moist = 1023 - moist;
  //moist = 300;
  //Serial.println(moist);
  
                                                                                      //濕度小於200澆水
  if(changeMode == 0 && hour2 == Clock.getHour(h12, PM) && minute2 == Clock.getMinute() && today == 0 && moist < 200)
  {
    water: //goto 的標籤
    
    int dot = 0;
    lcd.clear();
    digitalWrite(watering, LOW);
    changeMode = 5;
    lcd.setCursor(2, 1);
    lcd.print("Auto Watering");
    lcd.setCursor(8, 2);
    for(int i = 0; i<waterAmount; i++)
    {
      if(dot == 3)
      {
        lcd.setCursor(8, 2);
        lcd.print("   ");
        lcd.setCursor(8, 2);
        dot = 0;
        
      }
      dot++;   
      lcd.print(".");
      delay(1000);
    }
    digitalWrite(watering, HIGH);
    lcd.clear();
    delay(100);
    changeMode = 0;
    today = 1;  //表示已澆過一次水了
  }
  else if(changeMode == 0 && hour2 == Clock.getHour(h12, PM) && minute2 == Clock.getMinute() && today == 0 && moist > 200)
  {
    if(detect == 2) //檢測幾後如果還沒到標準之下 強制澆水 
    {
      goto water;
    }
    //hour2 = hour2 + 1; //檢測後如不需要澆水 就延長1小時
    minute2 = minute2 + 1;
    detect = detect + 1;
    
 
    lcd.setCursor(10, 0);
    lcd.print("+");
    lcd.print(detect);
    lcd.print("h");
  
  }
}
void Upload()
{ 
  if(!client.connected())   //上傳濕度值
  {
    client.connect(server, 80);
    val = moist;
    upload = "GET /update?api_key=MJBM2SXT7ZN4A9JZ&field1=" + String(val);
    client.println(upload);
  }
  
  val = moist;
  upload = "GET /update?api_key=MJBM2SXT7ZN4A9JZ&field1=" + String(val);
  client.println(upload);
  
}
void DownLoad()
{
  if(!client2.connected())
  {
    if(client2.connect(server,80))
    upload2 = "GET /channels/734036/fields/3/last.txt";
    client2.println(upload2);
  }
  while(client2.available())
  {
    temp = client2.read();
    Serial.println(temp);
    if(temp == '1' && rr == 0 && moist < 200)    //讀到遠端遙控值為1 (啟動澆水)   //案澆水時會一直讀到1 為了怕一直澆水 設定一個變數來
    {
      changeMode = 1;
      option = 0;
      delay(300);
      rr = 1;
    }else if(temp == '0')
    {
      rr=0;
    }
  }
}
void Number()
{
  Serial.print("waterAmount: ");
  Serial.println(waterAmount);
  Serial.print("changeMode: ");
  Serial.println(changeMode);
  Serial.print("option: ");
  Serial.println(option);
  Serial.print("arrow: ");
  Serial.println(arrow);
  Serial.println();
}
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  pinMode(moistSensor, INPUT);
  pinMode(watering, OUTPUT);
  pinMode(NoWaterWarning, OUTPUT);
  pinMode(5, INPUT);  //確定
  pinMode(6, INPUT);  //切換
  pinMode(7, INPUT);  //上
  pinMode(8, INPUT);  //下
  pinMode(9, INPUT);  //返回
  
  digitalWrite(watering, HIGH);//一開始要設定高電壓 不然會啟動馬達
  //Clock_default(); //重製時間用 不要動
  
  lcd.begin();
  for(int i = 0; i<3; i++)
  { 
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Hello Maker!!");
  lcd.setCursor(0, 2);
  delay(2000);
  lcd.print("Welcome to");
  lcd.setCursor(0, 3);
  lcd.print("Auto watering~~");
  delay(5000);
  lcd.clear();

  //////////連線部分////////////
  lcd.setCursor(4,1);
  lcd.print("Connecting");
  Link=WiFi.begin(ssid, pass);
  if(Link == 3){
    Serial.println("連線成功");
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("Connect Successful!");
    delay(2000);
    lcd.clear();
  }

  while(Link == 4){
    Link=WiFi.begin(ssid, pass);
    if(a == 10){
      Serial.println ("取消連線");
      lcd.clear();
      lcd.setCursor(1,1);
      lcd.print("Connect fail");
      delay(1500);
      lcd.clear();
      break;
    }
    if(Link == 3){
     Serial.println("連線成功");
     lcd.clear();
     lcd.setCursor(1,1);
     lcd.print("Connect Successful!");
     delay(2000);
     lcd.clear();
     break;
    }
    int dot = 0;
    lcd.setCursor(7, 2);
    for(int i=0;i <a ;i++)   //設定澆水量
    {
      if(dot == 3) 
      { 
        lcd.setCursor(7, 2);
        lcd.print("   ");
        lcd.setCursor(7, 2);
        dot = 0;
      }
      dot++;
      lcd.print(".");
      delay(500);        
    }
 
    a = a + 1;   
  }

 
}

void loop()
{
  Input();
  Functional();
  ReadDS3231();
  AutoWatering();
  Upload();
  DownLoad();
  //Number();
  
}
