{
 Camera Data..
 1.14.2024 ~q
}
unit uCamServerData;

interface

uses
  System.SysUtils, System.Classes, System.SyncObjs,System.Generics.Collections,
  System.Generics.Defaults,System.IOUtils;

const
   MAX_CAMS  = 99;
   FILE_NAME = 'EspCamServer.dat';

type
    TCamId = packed record
      camIdent     : array[0..39] of byte;
      camNum       : byte;
      VerLO   : byte;
      VerHI   : byte;
    end;


type  //32 bytes..
   TCamSensor = packed record
     FrameSize    :int32;//resolution enum
     Scale        :byte;//bool??
     Binning      :byte;//bool??
     Quality      :byte;//0 - 63
     Brightness   :ShortInt;//-2 - 2
     Contrast     :ShortInt;//-2 - 2
     Saturation   :ShortInt;//-2 - 2
     Sharpness    :ShortInt;//-2 - 2
     Denoise      :byte;//bool
     SpecialEffect:byte;//0 - 6
     WbMode       :byte;//0 - 4
     Awb          :byte;//bool
     AwbGain      :byte;//bool
     Aec          :byte;//bool
     Aec2         :byte;//bool
     AeLevel      :ShortInt;//-2 - 2
     AecValue     :word;//0 - 1200
     Agc          :byte;//bool
     AgcGain      :byte;//0 - 30
     GainCeiling  :byte;//0 - 6
     Bpc          :byte;//bool
     Wpc          :byte;//bool
     Raw_gma      :byte;//bool
     Lenc         :byte;//bool
     HorzMirror   :byte;//bool
     VertFlip     :byte;//bool
     Dcw          :byte;//bool
     Colorbar     :byte;//bool
     CapInterval  :UINT32;
     debug        :byte;
  end;

type
  TCamRecordOpts = packed record
    recMode:byte;
    recInterval:UINT32;
    remoteRecord:boolean;
    remoteAlert:boolean;
  end;

type
  TCamWndProps = packed record
    WndLeft  :integer;
    WndTop   :integer;
    WndHeight:integer;
    WndWidth :integer;
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
     //readers and writers
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
     procedure SetRecMod(amod:byte);
     function  GetRecMod:byte;
     procedure SetRecInt(aInterval:UINT32);
     function  GetRecInt:UINT32;
     procedure SetRemRec(aRem:boolean);
     function  GetRemRec:boolean;
     procedure SetRemAlt(aRem:boolean);
     function  GetRemAlt:boolean;
    public
     constructor Create;
     destructor  Destroy;override;
     //load and save
     procedure   SetRec(rec:TCamRec);
     function    GetRec:TCamRec;
     procedure   NewRec(num:byte);
     //props..
     property    CamId      :TGuid      read GetId     write SetId;
     property    CamNum     :byte       read GetCamNum write SetCamNum;
     property    CapDelay   :word       read GetCaptD  write SetCaptD;
     property    FirmVerLo  :byte       read GetFwLo   write SetFwLo;
     property    FirmVerHi  :byte       read GetFwHi   write SetFwHi;
     property    Top        :integer    read GetTop    write SetTop;
     property    Left       :integer    read GetLeft   write SetLeft;
     property    Height     :integer    read GetHeight write SetHeight;
     property    Width      :integer    read GetWidth  write SetWidth;
     property    Sensor     :TCamSensor read GetSensor write SetSensor;
     property    RecMode    :byte       read GetRecMod write SetRecMod;
     property    RecIntv    :UINT32     read GetRecInt write SetRecInt;
     property    RemoteRecrd:boolean    read GetRemRec write SetRemRec;
     property    RemoteAlert:boolean    read GetRemAlt write SetRemAlt;
  end;



  type
    TEspCamServerData = class
      private
       fCrit    :TCriticalSection;
       fCamCount:integer;
       fCamz     :TList<TEspCamera>;
       fPath:string;
      protected
       function  GetCamCount:integer;
       function  GetCam(index:integer):TEspCamera;
       procedure SetCam(index:integer;cam:TEspCamera);
      public
       constructor Create;
       destructor  Destroy;override;
       //pub meths..
       function    Load               :boolean;
       function    Save               :boolean;
       function    AddCam             :integer;
       function    DelCam(ndx:integer):boolean;
       function    FindCam(aID:TGuid) :integer;
       //props
       property    Path              :string     read fPath write fPath;
       property    CamCount          :integer    read GetCamCount;
       property    Cam[index:integer]:TEspCamera read GetCam write SetCam;
    end;


//global var..
var
   CamSrvrDat:TEspCamServerData;


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
var
aStr:String;
aBytes:TBytes;
begin
  //load from a record..
  move(rec,fCamRec,sizeof(TCamRec));
 // fCamRec := rec;
  //update the guid..
    SetLength(aBytes,SizeOf(rec.camId.camIdent));
    move(rec.camId.CamIdent[0],aBytes[0],SizeOf(rec.camId.camIdent));
    aStr:=TEncoding.ASCII.GetString(aBytes);
    aStr:=Trim(aStr);
    try
      fId := StringToGUID(aStr);
    finally
      SetLength(aBytes,0);
    end;


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
  fCamRec.camWnd.WndLeft     := 100;
  fCamRec.camWnd.WndTop      := 100;
  fCamRec.camWnd.WndHeight   := 300;
  fCamRec.camWnd.WndWidth    := 400;
  fCamRec.camRecOpts.recMode := 0;
  fCamRec.camRecOpts.recInterval := 1000;

  SetLength(aBytes,0);
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
  fCamRec.camSensor.CapInterval:=aDelay;
end;

function TEspCamera.GetCaptD: Word;
begin
  result := fCamRec.camSensor.CapInterval;
end;

procedure TEspCamera.SetFwLo(aLo:byte);
begin
  //lo
  fCamRec.camId.VerLO := aLo;
end;
function  TEspCamera.GetFwLo:byte;
begin
  //lo
  result:=fCamRec.camId.VerLO;
end;
procedure TEspCamera.SetFwHi(aHi:byte);
begin
  //hi
  fCamRec.camId.VerHI:=aHi;
end;
function  TEspCamera.GetFwHi:byte;
begin
  //hi
  result := fCamRec.camId.VerHI;
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
  fCamRec.camWnd.WndWidth:=awidth;
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

procedure TEspCamera.SetRecMod(amod: Byte);
begin
  fCamRec.camRecOpts.recMode:=amod;
end;

function TEspCamera.GetRecMod: Byte;
begin
  result:=fCamRec.camRecOpts.recMode;
end;

procedure TEspCamera.SetRecint(aInterval: UINT32);
begin
  fCamRec.camRecOpts.recInterval:=aInterval;
end;

function TEspCamera.GetRecInt: UINT32;
begin
  result:=fCamRec.camRecOpts.recInterval;
end;

procedure TEspCamera.SetRemRec(aRem: Boolean);
begin
  fCamRec.camRecOpts.remoteRecord:=aREm;
end;

function TEspCamera.GetRemRec: Boolean;
begin
  result:=fCamRec.camRecOpts.remoteRecord;
end;

procedure TEspCamera.SetRemAlt(aRem: Boolean);
begin
  fCamRec.camRecOpts.remoteAlert:=aRem;
end;

function TEspCamera.GetRemAlt: Boolean;
begin
  result:=fCamRec.camRecOpts.remoteAlert;
end;

// ** Cam Server Data **

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

//load the data
function  TEspCamServerData.Load: Boolean;
var
aFileName:String;
aDatFile:file of TCamRec;
aRec:TCamRec;
aCam:TEspCamera;
begin
//load data
  result := false;
  if fPath = '' then exit;

  aFileName := fPath+FILE_NAME;
  if TFile.Exists(aFileName) then
    begin
    AssignFile(aDatFile,aFileName);
    Reset(aDatFile);
     while not Eof(aDatFile) do
       begin
       try
        Read(aDatFile,aRec);
        aCam:=TEspCamera.Create;
        aCam.SetRec(aRec);
        fCamz.Add(aCam);
       finally
        ;
       end;
       end;
    CloseFile(aDatFile);
    end;

result := true;



end;

//save the data
function TEspCamServerData.Save: Boolean;
var
aFileName:String;
aDatFile:TFileStream;
aRec:TCamRec;
aCam:TEspCamera;
i:integer;
begin
  result := false;
  if fPath = '' then exit;
  aFileName := fPath+FILE_NAME;
 try
   aDatFile:=TFileStream.Create(TPath.Combine(fPath,FILE_NAME),fmCreate);
  except on e:EFCreateError do exit;   //oops, we tried..
  end;
    if fCamz.Count > 0 then
      begin
        for I := 0 to fCamz.Count -1 do
          begin
            aRec := fCamz[i].GetRec;
            aDatFile.Write(aRec,sizeof(aRec));
          end;
      end;
   aDatFile.Free;
    result := true;
end;


function  TEspCamServerData.GetCamCount: Integer;
begin
  fCrit.Enter;
     result := fCamz.Count;
  fCrit.Leave;
end;

function TEspCamServerData.GetCam(index:integer): TEspCamera;
begin
result := TEspCamera.Create;
 fCrit.Enter;
 result.SetRec(fCamz[index].GetRec);
 fCrit.Leave;
end;

procedure TEspCamServerData.SetCam(index: Integer; cam: TEspCamera);
begin
 fCrit.Enter;
  fCamz[index].SetRec(cam.GetRec);
 fCrit.Leave;
end;

function TEspCamServerData.AddCam: Integer;
var
aCam:TEspCamera;
begin
result := -1;
  //add a cam..
 fCrit.Enter;
   try
    aCam:=TEspCamera.Create;
    aCam.NewRec(fCamz.Count+1);
    result:=fCamz.Count;
    fCamz.Add(aCam);
   finally
    fCrit.Leave;
   end;
end;

function TEspCamServerData.DelCam(ndx: Integer): Boolean;
begin
  //delete a cam..
  result := false;
  fCrit.Enter;
 try
  if (ndx < fCamz.Count) and (fCamz.Count>0) then
    begin
    fCamz[ndx].Free;
    fCamz.Delete(ndx);
    result := true;
    end;
 finally
  fCrit.Leave;
 end;
end;

function TEspCamServerData.FindCam(aID: TGUID): Integer;
var
i:integer;
begin
  result := -1;
    fCrit.Enter;
    try
    if fCamz.Count> 0 then
      begin
       for I := 0 to fCamz.Count-1 do
        begin
          if IsEqualGUID(aID,fCamz[i].fId) then
           begin
             result := i;
             break;
           end;
        end;
      end;
    finally
    fCrit.Leave;
    end;
end;


initialization
begin
 CamSrvrDat:=TEspCamServerData.Create;
end;

finalization
begin
 try
  CamSrvrDat.Save;
 finally
 CamSrvrDat.Destroy;
 end;
end;


end.
