unit frmMain;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, OverbyteIcsWndControl, OverbyteIcsWSocket, Vcl.StdCtrls, Vcl.ExtCtrls,IpHlpApi,IpTypes;

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
    btnStart: TButton;
    memDisplay: TMemo;
    btnClose: TButton;
    sockUdp: TWSocket;
    tmrSend: TTimer;
    procedure sockUdpDataAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure btnStartClick(Sender: TObject);
    function  CheckCRC:boolean;
    procedure CheckSeq(packet:tUDPdatagram);
    procedure sockUdpChangeState(Sender: TObject; OldState, NewState: TSocketState);
    procedure btnCloseClick(Sender: TObject);
    procedure tmrSendTimer(Sender: TObject);
    procedure DiscoverMACIP;
    procedure FormClose(Sender: TObject; var Action: TCloseAction);


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
  ServerIP:string;
  ServerMAC:string;

implementation

{$R *.dfm}




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



procedure tMainFrm.DiscoverMACIP;
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


procedure TMainFrm.btnCloseClick(Sender: TObject);
begin
Close;
end;

procedure TMainFrm.btnStartClick(Sender: TObject);
begin
if btnStart.Tag=0 then
  begin
  sockUDP.Port:=edPort.Text;
  sockUDP.Connect;
  tmrSend.Enabled:=true;
  btnStart.Caption:='Stop';
  btnStart.Tag := 1;
  end else
     begin
     sockUDP.Close;
     tmrSend.Enabled:=false;
     btnStart.Caption:='Start';
     btnStart.Tag := 0;
     end;
end;

procedure TMainFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
tmrSend.Enabled:=false;
sockUDP.Close;
end;

procedure TMainFrm.FormCreate(Sender: TObject);
var
broadcast:string;
begin
SetLength(buff,sizeOf(tUDPdatagram));
recvCount:=0;
LastSeq:=0;
DiscoverMACIP;
if ServerIP<>'' then
  begin
    memDisplay.Lines.Insert(0,'IP: '+ServerIP);
    broadcast := IpToBroadcast(ServerIP);
    memDisplay.Lines.Insert(0,'Broadcast: '+broadcast);
  end;

 sockUDP.Addr:=broadcast;



end;

procedure TMainFrm.sockUdpChangeState(Sender: TObject; OldState, NewState: TSocketState);
begin
if NewState = TSocketState.wsListening then
  begin
  btnStart.Caption:='Stop';
  btnStart.Tag:=1;
  end else
if NewState = TSocketState.wsClosed then
  begin
  btnStart.Caption:='Start';
  btnStart.Tag:=0;
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
           memDisplay.Lines.Insert(0,'<< '+DateTimeToStr(NOW)+' seq:'+IntToStr(aPacket.seq)+' sdata:'+IntToStr(aPacket.sdata)+
           ' idata:'+IntToStr(aPacket.idata)+' fdata:'+FloatToStr(aPacket.fData)+' crc:'+IntToStr(aPacket.crc));
          end else
            memDisplay.Lines.Insert(0,DateTimeToStr(NOW)+' CRC Failed.');
         recvCount:=0;
        end;
end;

procedure TMainFrm.tmrSendTimer(Sender: TObject);
var
aPacket:tUDPdatagram;
  I: Integer;
begin
//transmitt a packet every 5 seconds..

Inc(LastSeq);
aPacket.seq:=LastSeq;
aPacket.sdata:=10;
aPacket.idata:=100;
aPacket.fData:=3.14;
aPacket.crc:=0;
move(aPacket,buff[0],SizeOf(tUDPdatagram));
for I := Low(buff) to (High(buff)-2) do
aPacket.crc:=aPacket.crc+buff[i];
move(aPacket,buff[0],SizeOf(tUDPdatagram));

memDisplay.Lines.Insert(0,'>> '+DateTimeToStr(NOW)+' seq:'+IntToStr(aPacket.seq)+' sdata:'+IntToStr(aPacket.sdata)+
           ' idata:'+IntToStr(aPacket.idata)+' fdata:'+FloatToStr(aPacket.fData)+' crc:'+IntToStr(aPacket.crc));
//send packet to local broadcast address..
sockUDP.Send(buff,SizeOf(tUDPdatagram));

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
