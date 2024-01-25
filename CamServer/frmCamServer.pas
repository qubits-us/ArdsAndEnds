unit frmCamServer;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtCtrls, Vcl.StdCtrls,
  OverbyteIcsWndControl, OverbyteIcsWSocket,frmCamView, Vcl.ComCtrls,System.IniFiles,IpHlpApi,IpTypes,
  Vcl.Menus,uCamServerData;

type
   tCamWndSettings = packed record
     left : integer;
     top : integer;
     Height: integer;
     Width: integer;
   end;

 //UDP discovery packet..
type
     TPacketDiscovery = packed record
       PackIdent :array[0..1] of byte;
       Port      :UINT16;
       Ip        :array[0..19] of byte;
     end;


type
  TCamServerFrm = class(TForm)
    pnlTop: TPanel;
    sckServer: TWSocket;
    btnCtrl: TButton;
    edPort: TEdit;
    lblPort: TLabel;
    sbMain: TStatusBar;
    btnPlayback: TButton;
    MainMenu1: TMainMenu;
    File1: TMenuItem;
    Exit1: TMenuItem;
    Cams1: TMenuItem;
    List1: TMenuItem;
    Firmwares1: TMenuItem;
    Help1: TMenuItem;
    About1: TMenuItem;
    sckDiscv: TWSocket;
    tmrDiscv: TTimer;
    procedure btnCtrlClick(Sender: TObject);
    procedure sckServerSessionAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure btnPlaybackClick(Sender: TObject);
    procedure About1Click(Sender: TObject);
    procedure Exit1Click(Sender: TObject);
    procedure tmrDiscvTimer(Sender: TObject);

  private
    { Private declarations }
     procedure   WMUser(var msg: TMessage); message WM_USER;
     procedure StartServer;
     procedure StopServer;
     procedure DiscoverMACIP;
  public
    { Public declarations }
  end;

var
  CamServerFrm: TCamServerFrm;
  Connections: integer;
  CamSettings:array of tCamWndSettings;
  //server properties..
  ServerIP:String;
  ServerBroadCastIP:String;
  ServerMAC:String;
  ServerName:String;
  DiscvPack :TPacketDiscovery;


implementation

{$R *.dfm}

uses frmReplay;

function GetCompName: string;
var
  buffer: array[0..MAX_COMPUTERNAME_LENGTH + 1] of Char;
  Size: Cardinal;
begin
  Size := MAX_COMPUTERNAME_LENGTH + 1;
  GetComputerName(@buffer, Size);
  Result := StrPas(buffer);
end;

function IpToBroadcast(ipStr:String):String;
var
aCount:integer;
aPos:integer;
begin
result:='';
//ipStr should be like ###.###.###.###
//first block
aPos:=Pos('.',ipStr);
if aPos>0 then
result:=Copy(ipStr,1,aPos);
Delete(ipStr,1,aPos);
//second block
aPos:=Pos('.',ipStr);
if aPos>0 then
result:=result+Copy(ipStr,1,aPos);
Delete(ipStr,1,aPos);
//third block
aPos:=Pos('.',ipStr);
if aPos>0 then
result:=result+Copy(ipStr,1,aPos);
Delete(ipStr,1,aPos);
result:=result+'255';
end;



procedure TCamServerFrm.DiscoverMACIP;
var
  pAdapterInfo, pTempAdapterInfo: PIP_ADAPTER_INFO;
  BufLen: DWORD;
  Status: DWORD;
  strMAC: String;
  i: Integer;
begin

  BufLen := SizeOf(IP_Adapter_Info);
  GetMem(pAdapterInfo, BufLen);
  try
    repeat
      Status := GetAdaptersInfo(pAdapterInfo, BufLen);
      if (Status = ERROR_SUCCESS) then
      begin
        if BufLen <> 0 then Break;
        Status := ERROR_NO_DATA;
      end;
      if (Status = ERROR_BUFFER_OVERFLOW) then
      begin
        ReallocMem(pAdapterInfo, BufLen);
      end else
      begin
        case Status of
          ERROR_NOT_SUPPORTED:
            ;//DisplayMemo.Lines.Add('GetAdaptersInfo is not supported by the operating system running on the local computer.');
          ERROR_NO_DATA:
           ;// DisplayMemo.Lines.Add('No network adapter on the local computer.');
        else
           ;// DisplayMemo.Lines.Add('GetAdaptersInfo failed with error #' + IntToStr(Status));
        end;
        Exit;
      end;
    until False;

    pTempAdapterInfo := pAdapterInfo;
    while (pTempAdapterInfo <> nil) do
    begin
      strMAC := '';
      for I := 0 to pTempAdapterInfo^.AddressLength - 1 do
        strMAC := strMAC + '-' + IntToHex(pTempAdapterInfo^.Address[I], 2);
      Delete(strMAC, 1, 1);
      if pTempAdapterInfo^.IpAddressList.IpAddress.S<>'0.0.0.0' then
        begin
          ServerIp:=pTempAdapterInfo^.IpAddressList.IpAddress.S;
          ServerMAC:=StrMAC;
        end;
      pTempAdapterInfo := pTempAdapterInfo^.Next;
    end;
  finally
    FreeMem(pAdapterInfo);
  end;
end;






procedure TCamServerFrm.btnPlaybackClick(Sender: TObject);
var
frm:TReplayFrm;
begin
//open the playback window..
frm :=tReplayFrm.Create(application);
frm.ShowModal;
frm.Free;

end;

procedure TCamServerFrm.Exit1Click(Sender: TObject);
begin
Close;
end;

procedure TCamServerFrm.FormClose(Sender: TObject; var Action: TCloseAction);
var
ini:tIniFile;
NumCams, i, port:integer;
begin

//shutdown discovery..
tmrDiscv.Enabled := false;
sckDiscv.Close;

//all done
ini := TIniFile.Create(ChangeFileExt(Application.ExeName,'.ini'));
NumCams:=length(CamSettings);
ini.WriteInteger('General','CamCount',NumCams);
port :=StrToInt(sckServer.Port);
if port = 0 then port := 50001;

Ini.WriteInteger('General','Port',sckServer.PortNum);


if NumCams > 0 then
  begin
    //save cam window settings..
    for I := 0 to NumCams-1 do
      begin
        ini.WriteInteger('Cam'+IntToStr(i+1),'left',CamSettings[i].left);
        ini.WriteInteger('Cam'+IntToStr(i+1),'top',CamSettings[i].top);
        ini.WriteInteger('Cam'+IntToStr(i+1),'width',CamSettings[i].width);
        ini.WriteInteger('Cam'+IntToStr(i+1),'height',CamSettings[i].height);
      end;
  end;

ini.Free;


end;

procedure TCamServerFrm.FormCreate(Sender: TObject);
var
ini :TIniFile;
NumCams, port:integer;
aPath:String;
begin

DiscoverMACIP;
ServerBroadcastIP := ipToBroadcast(ServerIP);
if ServerBroadcastIP = '' then ServerBroadcastIP :='255.255.255.255';

//load settings..

ini := TIniFile.Create(ChangeFileExt(Application.ExeName,'.ini'));
NumCams:=ini.ReadInteger('General','CamCount',0);
port:=Ini.ReadInteger('General','Port',50001);

if port = 0 then port := 50001;

edPort.Text := IntToStr(port);
ini.Free;


aPath:=ExtractFilePath(Application.ExeName);

CamSrvrDat.Path := aPath;
CamSrvrDat.Load;

sckServer.Port := IntToStr(port);

//udp discovery socket..
sckDiscv.Addr := ServerBroadcastIP;
sckDiscv.Port := '50000';
sckDiscv.Connect;


aPath:=aPath+'video';
{$I-}
  MkDir(aPath);
{$I+}


Left := 10;
Top := 10;


end;

procedure TCamServerFrm.sckServerSessionAvailable(Sender: TObject;
  ErrCode: Word);
var
form :TCamViewFrm;
begin
//incoming..
Inc(Connections);
sbMain.Panels[0].Text:='Connections: '+IntToStr(Connections);
//spawn a new cam view window..
form := tCamViewFrm.Create(self);
form.CamNum := Connections;
form.Caption := 'ESP Cam : '+IntToStr(Connections);
form.vFilePath := form.vFilePath;
//create the cam root folder..
{$I-}
  MkDir(form.vFilePath);
{$I+}

//now accept the new client connection..
form.sckCam.HSocket := sckServer.Accept;
//default pos until known..
form.Left   := Screen.Width -500;
form.top    := 100;
form.width  := 200;
form.height := 200;
//make it visible..
form.Show;

end;

procedure TCamServerFrm.StartServer;
var
tmpBytes:tBytes;

begin
  //start the server..
    sckServer.Close;
    sckServer.Addr  := '0.0.0.0';
    sckServer.Port  := edPort.Text;
    sckServer.Proto := 'tcp';
    sckServer.Listen;
     //start broadcasting..
    DiscvPack.PackIdent[0] := IDENT_LO;
    DiscvPack.PackIdent[1] := IDENT_HI;
    DiscvPack.Port:=StrToInt(sckServer.Port);
    FillChar(DiscvPack.Ip,SizeOf(DiscvPack.Ip),#0);//clear
    tmpBytes := TEncoding.ASCII.GetBytes(ServerIP);
 if (Length(tmpBytes) >0) AND (Length(tmpBytes) < SizeOf(DiscvPack.Ip)) then
  begin
   Move(tmpBytes[0],DiscvPack.Ip,Length(tmpBytes));
   //discovery packet prepped, activate timer..
   tmrDiscv.Enabled := true;
   //ShowMessage('Discovery enabled');
  end;
  SetLength(tmpBytes,0);

end;

Procedure TCamServerFrm.StopServer;
begin
  //stop the server..
  sckServer.Close;
  //stop broadcasting..
  tmrDiscv.Enabled := false;
end;


procedure TCamServerFrm.tmrDiscvTimer(Sender: TObject);
var
tmpBytes:tBytes;
begin
tmrDiscv.Enabled := false;
//send out a discovery packet..
sckDiscv.Send(@DiscvPack,SizeOf(TPacketDiscovery));
//reset timer for next shot..
tmrDiscv.Enabled := true;
end;

procedure TCamServerFrm.WMUser(var msg: TMessage);
var
Form:TCamViewFrm;
CamNum:integer;
begin
    Form:=TCamViewFrm(Msg.lParam);
    Form.Release;  //free cam view form..
    if Connections >0 then
      begin
        Dec(Connections);
        sbMain.Panels[0].Text:='Connections: '+IntToStr(Connections);
      end;
end;

procedure TCamServerFrm.About1Click(Sender: TObject);
begin
ShowMessage('ESP32 Cam Server'+#10+#13+'2024 qubits.us');
end;

procedure TCamServerFrm.btnCtrlClick(Sender: TObject);
begin
if btnCtrl.Tag = 0 then
  begin
    //start listening..
    StartServer;
    btnCtrl.Tag:=1;
    btnCtrl.Caption:='Stop';
  end else
    begin
    //stop listening..
    StopServer;
    btnCtrl.Tag:=0;
    btnCtrl.Caption:='Listen';
    end;
end;

end.
