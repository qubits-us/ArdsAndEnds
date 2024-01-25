{
 Camera Data..
 1.14.2024
}
unit uCamData;

interface

uses
  System.SysUtils, System.Classes, System.SyncObjs,System.Generics.Collections,System.Generics.Defaults,System.Hash;

const
   MAX_CAMS = 99;

type  //30 bytes..
   TCamSensor = packed record
    FrameSize:byte;//resolution enum
    Scale:byte;//bool??
    Binning:byte;//bool??
    Quality:word;//0 - 63
    Brightness:ShortInt;//-2 - 2
    Contrast:ShortInt;//-2 - 2
    Saturation:ShortInt;//-2 - 2
    Sharpness:ShortInt;//-2 - 2
    Denoise:byte;//bool
    SpecialEffect:byte;//0 - 6
    WbMode:byte;//0 - 4
    Awb:byte;//bool
    AwbGain:byte;//bool
    Aec:byte;//bool
    Aec2:byte;//bool
    AeLevel:ShortInt;//-2 - 2
    AecValue:word;//0 - 1200
    Agc:byte;//bool
    AgcGain:byte;//0 - 30
    GainCeiling:byte;//0 - 6
    Bpc:byte;//bool
    Wpc:byte;//bool
    Raw_gma:byte;//bool
    Lenc:byte;//bool
    HorzMirror:byte;//bool
    VertFlip:byte;//bool
    Dcw:byte;//bool
    Colorbar:byte;//bool
  end;

type
  TCamRecordOpts = packed record
    recMode:byte;
    recInterval:word;
  end;

type
  TCamWndProps = packed record
    WndLeft  :integer;
    WndTop   :integer;
    WndHeight:integer;
    WndWidth :integer;
  end;

  type
    TCamId = packed record
      camIdent     : array[0..39] of byte;
      camNum       : byte;
      camCaptDelay : word;
      camFWverLO   : byte;
      camFWverHI   : byte;
    end;


type
  TCamRec = Packed record
    camId     :TCamId;
    camSensor :TCamSensor;
    camWnd    :TCamWndProps;
    camRecOpts:TCamRecordOpts;
  end;



type
  TEspCamera = class
    private
     fCamRec : TCamRec;
     fId:tGuid;
    protected
     //readers and wrtiers
     procedure SetId(id:TGuid);
     function  GetId:TGuid;
     procedure SetCamNum(aNum:byte);
     function  GetCamNum:byte;
     procedure SetCaptD(aDelay:word);
     function  GetCaptD:word;
     procedure SetFwLo(aLo:byte);
     function  GetFwLo:byte;
     procedure SetFwHi(aHi:byte);
     function  GetFwHi:byte;
     procedure SetTop(atop:integer);
     function  GetTop:integer;
     procedure SetLeft(aleft:integer);
     function  GetLeft:integer;
     procedure SetHeight(aheight:integer);
     function  GetHeight:integer;
     procedure SetWidth(awidth:integer);
     function  GetWidth:integer;
     procedure SetSensor(asensor:TCamSensor);
     function  GetSensor:TCamSensor;
    public
     constructor Create;
     destructor  Destroy;override;
     //load and save
     procedure   SetRec(rec:TCamRec);
     function    GetRec:TCamRec;
     procedure   NewRec(num:byte);
     //props..
     property    CamId    :TGuid      read GetId     write SetId;
     property    CamNum   :byte       read GetCamNum write SetCamNum;
     property    CapDelay :word       read GetCaptD  write SetCaptD;
     property    FirmVerLo:byte       read GetFwLo  write SetFwLo;
     property    FirmVerHi:byte       read GetFwHi  write SetFwHi;
     property    Top      :integer    read GetTop    write SetTop;
     property    Left     :integer    read GetLeft   write SetLeft;
     property    Height   :integer    read GetHeight write SetHeight;
     property    Width    :integer    read GetWidth  write SetWidth;
     property    Sensor   :TCamSensor read GetSensor write SetSensor;
  end;



  type
    TEspCamServerData = class
      private
       fCrit:TCriticalSection;
       fCamCount:integer;
       fCamz:TList<TEspCamera>;
      protected

      public
       constructor Create;
       destructor Destroy;override;
    end;




implementation


constructor TEspCamera.Create;
begin
  //create..
  fId := StringToGuid('{00000000-0000-0000-0000-000000000000}');
  fCamRec.camId.camNum :=0;
end;

destructor TEspCamera.Destroy;
begin
  //free.. nothing todo so far..
end;

procedure TEspCamera.SetRec(rec: TCamRec);
begin
  //load from a record..
  fCamRec := rec;
end;

function TEspCamera.GetRec: TCamRec;
begin
  result:=fCamRec;
end;

procedure TEspCamera.NewRec(num:byte);
var
aStr:String;
aBytes:TBytes;
begin
  //new cam..
  CreateGUID(fid);
  aStr  :=GUIDToString(fid);
  aBytes:=TEncoding.ASCII.GetBytes(aStr);
  if Length(aBytes) < sizeof(fCamRec.camId.camIdent) then
  Move(aBytes[0],fCamRec.camId.camIdent[0],length(aBytes));
  fCamRec.camId.camNum := num;
  fCamRec.camId.camCaptDelay := 100;
  fCamRec.camWnd.WndLeft := 100;
  fCamRec.camWnd.WndTop := 100;
  fCamRec.camWnd.WndHeight := 300;
  fCamRec.camWnd.WndWidth := 400;
end;

procedure TEspCamera.SetId(id:TGuid);
var
aStr:String;
aBytes:TBytes;
begin
 //set id to new id..
 if not isEqualGUID(id,fid) then
   begin
   fId:=id;
   aStr  :=GUIDToString(fid);
   aBytes:=TEncoding.ASCII.GetBytes(aStr);
   if Length(aBytes) < sizeof(fCamRec.camId.camIdent) then
   Move(aBytes[0],fCamRec.camId.camIdent[0],length(aBytes));
   end;
end;

function TEspCamera.GetId: TGuid;
begin
  //get the cams guid..
 result := fId;
end;

procedure TEspCamera.SetCamNum(aNum: Byte);
begin
  //set cam num
  fCamRec.camId.camNum:=aNum;
end;

function TEspCamera.GetCamNum: Byte;
begin
  //get cam num
  result:=fCamRec.camId.camNum;
end;

procedure TEspCamera.SetCaptD(aDelay: Word);
begin
  fCamRec.camId.camCaptDelay:=aDelay;
end;

function TEspCamera.GetCaptD: Word;
begin
  result := fCamRec.camId.camCaptDelay;
end;

procedure TEspCamera.SetFwLo(aLo:byte);
begin
  //lo
  fCamRec.camId.camFWverLO := aLo;
end;
function  TEspCamera.GetFwLo:byte;
begin
  //lo
  result:=fCamRec.camId.camFWverLO;
end;
procedure TEspCamera.SetFwHi(aHi:byte);
begin
  //hi
  fCamRec.camId.camFWverHI:=aHi;
end;
function  TEspCamera.GetFwHi:byte;
begin
  //hi
  result := fCamRec.camId.camFWverHI;
end;

procedure TEspCamera.SetTop(atop: Integer);
begin
  //top
  fCamRec.camWnd.WndTop:=aTop;
end;

function TEspCamera.GetTop: Integer;
begin
  //top
  result:=fCamRec.camWnd.WndTop;
end;


procedure TEspCamera.SetLeft(aleft: Integer);
begin
  //left
  fCamRec.camWnd.WndLeft:=aleft;
end;

function TEspCamera.GetLeft: Integer;
begin
  //left
  result:=fCamRec.camWnd.WndLeft;
end;

procedure TEspCamera.SetHeight(aheight: Integer);
begin
  //h
  fCamRec.camWnd.WndHeight:=aheight;
end;

function TEspCamera.GetHeight: Integer;
begin
  //h
  result:=fCamRec.camWnd.WndHeight;
end;

procedure TEspCamera.SetWidth(awidth: Integer);
begin
  //w
  fCamRec.camWnd.WndTop:=awidth;
end;

function TEspCamera.GetWidth: Integer;
begin
  //w
  result:=fCamRec.camWnd.WndWidth;
end;

procedure TEspCamera.SetSensor(asensor: TCamSensor);
begin
  //sensor
  fCamRec.camSensor := asensor;
end;

function TEspCamera.GetSensor: TCamSensor;
begin
  //get sensor
  result := fCamRec.camSensor;
end;



constructor TEspCamServerData.Create;
begin
  //create..
  fCrit := TCriticalSection.Create;
  fCamz := TList<TEspCamera>.Create;

end;

destructor TEspCamServerData.Destroy;
var
cam:TEspCamera;
begin
  //destroy..
  fCrit.Free;
  for cam in fCamz do
    begin
      cam.Free;
    end;
 fCamz.Clear;
 fCamz.Free;




end;



end.
