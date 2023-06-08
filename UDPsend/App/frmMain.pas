unit frmMain;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, OverbyteIcsWndControl, OverbyteIcsWSocket, Vcl.StdCtrls, Vcl.ExtCtrls;

{

// UDP Datagram
struct __attribute__((packed)) UDPDatagram
  uint16_t seq;                          // sequence number
  int16_t sdata;                         // 16 bit integer data
  int32_t idata;                         // 32 bit integer data
  float fdata;                           // float data
  uint16_t crc;                          // crc check

}

type
    tUDPDatagram = packed record
      seq  : UINT16;
      sdata: INT16;
      idata: INT32;
      fData: single;
      crc  : UINT16;
    end;




type
  TMainFrm = class(TForm)
    Panel1: TPanel;
    Panel2: TPanel;
    edPort: TEdit;
    lblPort: TLabel;
    btnListen: TButton;
    memDisplay: TMemo;
    btnClose: TButton;
    sockUdp: TWSocket;
    procedure sockUdpDataAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure btnListenClick(Sender: TObject);
    function  CheckCRC:boolean;
    procedure CheckSeq(packet:tUDPdatagram);
    procedure sockUdpChangeState(Sender: TObject; OldState, NewState: TSocketState);
    procedure btnCloseClick(Sender: TObject);


  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  MainFrm: TMainFrm;
  buff:tBytes;
  recvCount:integer;
  LastSeq:integer;

implementation

{$R *.dfm}

procedure TMainFrm.btnCloseClick(Sender: TObject);
begin
Close;
end;

procedure TMainFrm.btnListenClick(Sender: TObject);
begin
if btnListen.Tag=0 then
  begin
  sockUDP.Port:=edPort.Text;
  sockUDP.Listen;
  end else
     begin
     sockUDP.Close;
     end;
end;

procedure TMainFrm.FormCreate(Sender: TObject);
begin
SetLength(buff,sizeOf(tUDPdatagram));
recvCount:=0;
LastSeq:=0;

end;

procedure TMainFrm.sockUdpChangeState(Sender: TObject; OldState, NewState: TSocketState);
begin
if NewState = TSocketState.wsListening then
  begin
  btnListen.Caption:='Stop';
  btnListen.Tag:=1;
  end else
if NewState = TSocketState.wsClosed then
  begin
  btnListen.Caption:='Listen';
  btnListen.Tag:=0;
  end;

end;

procedure TMainFrm.sockUdpDataAvailable(Sender: TObject; ErrCode: Word);
var
len:integer;
aPacket:tUDPdatagram;
begin
//got some data
     Len:=sockUDP.Receive(@buff[recvCount],SizeOf(tUDPdatagram)-recvCount);
     recvCount:=recvCount+Len;
     if recvCount=SizeOf(tUDPdatagram) then
        begin
        if CheckCRC then
          begin
           move(buff[0],aPacket,SizeOf(tUDPdatagram));
           if memDisplay.Lines.Count>999 then
               memDisplay.Lines.Clear;
           CheckSeq(aPacket);
           memDisplay.Lines.Insert(0,DateTimeToStr(NOW)+' seq:'+IntToStr(aPacket.seq)+' sdata:'+IntToStr(aPacket.sdata)+
           ' idata:'+IntToStr(aPacket.idata)+' fdata:'+FloatToStr(aPacket.fData)+' crc:'+IntToStr(aPacket.crc));
          end else
            memDisplay.Lines.Insert(0,DateTimeToStr(NOW)+' CRC Failed.');
         recvCount:=0;
        end;
end;

procedure tMainFrm.CheckSeq(packet:tUDPdatagram);
begin
//quick sequence check
  if LastSeq = 0 then
    begin
      LastSeq:=packet.seq;
    end else
      begin
        if (packet.seq-1<>LastSeq) then
         memDisplay.Lines.Insert(0,DateTimeToStr(NOW)+' sequence deviation detected, possible packet loss..');
       LastSeq:=packet.seq;
      end;
end;

function TMainFrm.CheckCRC:boolean;
var
i:integer;
crc,check:word;

begin
crc:=0;
check:=buff[High(buff)] shl 8 or buff[High(buff)-1];

  for I := LOW(buff) to (High(buff)-2) do
        crc:=crc+ buff[i];

   if crc = check then result := true else
    begin
     result := false;
     memDisplay.Lines.Insert(0,'CRC check:'+IntToStr(check)+' CRC calc:'+intToStr(crc));
    end;
end;

end.
