unit frmCamServer;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtCtrls, Vcl.StdCtrls,
  OverbyteIcsWndControl, OverbyteIcsWSocket,frmCamView, Vcl.ComCtrls;

type
  TCamServerFrm = class(TForm)
    pnlTop: TPanel;
    sckServer: TWSocket;
    btnCtrl: TButton;
    txtPort: TEdit;
    lblPort: TLabel;
    sbMain: TStatusBar;
    procedure btnCtrlClick(Sender: TObject);
    procedure sckServerSessionAvailable(Sender: TObject; ErrCode: Word);
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

implementation

{$R *.dfm}


procedure TCamServerFrm.sckServerSessionAvailable(Sender: TObject;
  ErrCode: Word);
var
form :TCamViewFrm;
begin
//incoming..
Inc(Connections);
sbMain.Panels[0].Text:='Connections: '+IntToStr(Connections);
form :=tCamViewFrm.Create(self);
//now accept the new client connection..
form.sckCam.HSocket := sckServer.Accept;
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
begin

  try
    Form:=TCamViewFrm(Msg.lParam);
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
