unit frmCamServer;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtCtrls, Vcl.StdCtrls,
  OverbyteIcsWndControl, OverbyteIcsWSocket,frmCamView, Vcl.ComCtrls,System.IniFiles;

type
   tCamWndSettings = packed record
     left : integer;
     top : integer;
     Height: integer;
     Width: integer;
   end;

type
  TCamServerFrm = class(TForm)
    pnlTop: TPanel;
    sckServer: TWSocket;
    btnCtrl: TButton;
    txtPort: TEdit;
    lblPort: TLabel;
    sbMain: TStatusBar;
    btnPlayback: TButton;
    procedure btnCtrlClick(Sender: TObject);
    procedure sckServerSessionAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure btnPlaybackClick(Sender: TObject);
  private
    { Private declarations }
     procedure   WMUser(var msg: TMessage); message WM_USER;
     procedure StartServer;
     procedure StopServer;
  public
    { Public declarations }
  end;

var
  CamServerFrm: TCamServerFrm;
  Connections: integer;
  CamSettings:array of tCamWndSettings;

implementation

{$R *.dfm}

uses frmReplay;


procedure TCamServerFrm.btnPlaybackClick(Sender: TObject);
var
frm:TReplayFrm;
begin
//open the playback window..
frm :=tReplayFrm.Create(application);
frm.ShowModal;
frm.Free;

end;

procedure TCamServerFrm.FormClose(Sender: TObject; var Action: TCloseAction);
var
ini:tIniFile;
NumCams, i:integer;
begin
//all done
ini := TIniFile.Create(ChangeFileExt(Application.ExeName,'.ini'));
NumCams:=length(CamSettings);
ini.WriteInteger('General','CamCount',NumCams);

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
NumCams, i:integer;
aPath:String;
begin
//load settings..

ini := TIniFile.Create(ChangeFileExt(Application.ExeName,'.ini'));
NumCams:=ini.ReadInteger('General','CamCount',0);

if NumCams > 0 then
  begin
    //load cam window settings..
    setLength(CamSettings,NumCams);
    for I := 0 to NumCams-1 do
      begin
        CamSettings[i].left:=ini.ReadInteger('Cam'+IntToStr(i+1),'left',0);
        CamSettings[i].top:=ini.ReadInteger('Cam'+IntToStr(i+1),'top',0);
        CamSettings[i].width:=ini.ReadInteger('Cam'+IntToStr(i+1),'width',0);
        CamSettings[i].height:=ini.ReadInteger('Cam'+IntToStr(i+1),'height',0);
      end;
  end;

ini.Free;

aPath:=ExtractFilePath(Application.ExeName);
aPath:=aPath+'video';
{$I-}
  MkDir(aPath);
{$I+}

end;

procedure TCamServerFrm.sckServerSessionAvailable(Sender: TObject;
  ErrCode: Word);
var
form :TCamViewFrm;
begin
//incoming..
Inc(Connections);


sbMain.Panels[0].Text:='Connections: '+IntToStr(Connections);
form :=tCamViewFrm.Create(self);
form.CamNum:=Connections;
form.Caption:='ESP Cam : '+IntToStr(Connections);
//set path for video recording..
form.vFilePath:=form.vFilePath+'Cam'+IntToStr(Connections)+'\';
//create the cam root folder..
{$I-}
  MkDir(form.vFilePath);
{$I+}

//now accept the new client connection..
form.sckCam.HSocket := sckServer.Accept;
if Connections > length(CamSettings) then
  SetLength(CamSettings,Connections) else
    begin
      form.Left:=CamSettings[Connections-1].left;
      form.top:=CamSettings[Connections-1].top;
      form.width:=CamSettings[Connections-1].width;
      form.height:=CamSettings[Connections-1].height;
    end;


form.Show;

end;

procedure TCamServerFrm.StartServer;
begin
  //start the server..
    sckServer.Close;
    sckServer.Addr  := '0.0.0.0';
    sckServer.Port  := txtPort.Text;
    sckServer.Proto := 'tcp';
    sckServer.Listen;

end;

Procedure TCamServerFrm.StopServer;
begin
  //stop the server..
  sckServer.Close;

end;


procedure TCamServerFrm.WMUser(var msg: TMessage);
var
Form:TCamViewFrm;
CamNum:integer;
begin

  try
    Form:=TCamViewFrm(Msg.lParam);
    CamNum := Form.CamNum;
    if CamNum <=length(CamSettings) then
      begin
        //remember..
        CamSettings[CamNum-1].left:=Form.Left;
        CamSettings[CamNum-1].top:=Form.top;
        CamSettings[CamNum-1].width:=Form.width;
        CamSettings[CamNum-1].height:=Form.height;
      end;
    Form.Release;  //free cam view form..
    finally
     //
  end;

    if Connections >0 then
      begin
        Dec(Connections);
        sbMain.Panels[0].Text:='Connections: '+IntToStr(Connections);
      end;
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
