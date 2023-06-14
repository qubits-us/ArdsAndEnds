unit frmMain;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,System.IOUtils,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, OoMisc, AdPort, Vcl.ExtCtrls, Vcl.ComCtrls, AdPacket;

type
  TMainFrm = class(TForm)
    Panel1: TPanel;
    Panel2: TPanel;
    ComPort: TApdComPort;
    btnBegin: TButton;
    btnSelectFile: TButton;
    edPort: TEdit;
    edFileName: TEdit;
    lblPortNum: TLabel;
    lblFileName: TLabel;
    lblBaud: TLabel;
    cbBaud: TComboBox;
    pbProgress: TProgressBar;
    memDisplay: TMemo;
    edDelim: TEdit;
    lblDelim: TLabel;
    odSelectFile: TOpenDialog;
    DataPack: TApdDataPacket;
    tmrSend: TTimer;
    tmrResend: TTimer;
    tmrBegin: TTimer;
    procedure btnSelectFileClick(Sender: TObject);
    procedure btnBeginClick(Sender: TObject);
    procedure SendChunk;
    procedure Resend;
    procedure DataPackPacket(Sender: TObject; Data: Pointer; Size: Integer);
    procedure tmrSendTimer(Sender: TObject);
    procedure tmrResendTimer(Sender: TObject);
    procedure tmrBeginTimer(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  MainFrm: TMainFrm;
  FileBytes:TBytes;
  CurrentPos:integer;
  SendBuff:TBytes;
  Delim:Char;
  DelimCount:integer;

implementation

{$R *.dfm}

procedure TMainFrm.btnBeginClick(Sender: TObject);
var
aFileName,aDelim:String;
i:integer;
begin
//check file first..
aFileName:=edFileName.Text;
if aFileName = '' then
  begin
    ShowMessage('Please select a file..');
    exit;
  end;
aDelim:=edDelim.Text;
if aDelim='' then
  begin
    ShowMessage('Please enter a delimmiter..');
    exit;
  end;
  Delim:=aDelim[1];

if TFile.Exists(aFileName) then
 begin
     try
      FileBytes:=TFile.ReadAllBytes(aFileName);
     except on e:exception do
       begin
         ShowMessage(e.Message);
         SetLength(FileBytes,0);
         exit;
       end;
     end;
       //should have some bytes..
       DelimCount :=0;
       for I := Low(FileBytes) to High(FileBytes) do
           begin
             if FileBytes[i] = Ord(Delim) then
             Inc(DelimCount);
           end;
      if DelimCount = 0 then
         begin
           ShowMessage('No delimiters found, check file..');
           SetLength(FileBytes,0);
           exit;
         end;

         pbProgress.Max:=DelimCount;
         pbProgress.Position:=0;

         try
          ComPort.ComNumber:=StrToInt(edPort.Text);
          ComPort.Baud:=StrToInt(cbBaud.Text);
          ComPort.Open:=True;
          DataPack.Enabled:=true;
          except on e:Exception do
           begin
            ShowMessage(e.Message);
            SetLength(FileBytes,0);
            exit;
           end;
         end;

         //good to start..
         CurrentPos:=0;
         tmrBegin.Enabled:=true;
         btnBegin.Enabled:=false;

 end else
    begin
      ShowMessage('File not found..');
    end;



end;

procedure TMainFrm.SendChunk;
var
nextDelim,i,len:integer;
begin
  //


 if (CurrentPos + Length(SendBuff))>=Length(FileBytes) then
   begin
     //all done send..
     pbProgress.Position:=0;
     memDisplay.Lines.Insert(0,'Finished..');
     btnBegin.Enabled:=true;
     Comport.Open:=false;
     exit;
   end;

   nextDelim :=0;
  for I := CurrentPos to High(FileBytes) do
      begin
        if FileBytes[i] = Ord(Delim) then
          begin
            nextDelim := i;
            break;
          end
      end;

   if nextDelim > 0 then
     begin
       len:=nextDelim-CurrentPos;
      if len>0 then
       begin
       Inc(len);//want the delimiter too..
       SetLength(SendBuff,Len);
       move(FileBytes[CurrentPos],SendBuff[0],Len);
       Comport.PutBlock(SendBuff[0],Len);
       memDisplay.Lines.Insert(0,DateTimeToStr(Now)+' : >> Block:'+IntToStr(pbProgress.Position+1)+' len:'+IntToStr(Len));
       CurrentPos:=CurrentPos+Len;
       pbProgress.Position:=pbProgress.Position+1;
       end else
         begin
          memDisplay.Lines.Insert(0,'Empty Delimiter, skipping '+IntToStr(pbProgress.Position+1)+' Total Delims:'+IntToStr(DelimCount));
          CurrentPos:=CurrentPos+Len;
          pbProgress.Position:=pbProgress.Position+1;
          if pbProgress.Position<DelimCount then
          tmrSend.Enabled:=true;
         end;
     end else
       begin
         //all done..
       pbProgress.Position:=0;
       memDisplay.Lines.Insert(0,'Finished..');
       btnBegin.Enabled:=true;
       Comport.Open:=false;
       end;
end;

procedure TMainFrm.Resend;
begin
 if Length(SendBuff)>0 then
       Comport.PutBlock(SendBuff[0],Length(SendBuff));
end;

procedure TMainFrm.tmrBeginTimer(Sender: TObject);
begin
tmrBegin.Enabled:=false;
SendChunk;
end;

procedure TMainFrm.tmrResendTimer(Sender: TObject);
begin
tmrResend.Enabled:=false;
Resend;
end;

procedure TMainFrm.tmrSendTimer(Sender: TObject);
begin
tmrSend.Enabled:=false;
SendChunk;
end;

procedure TMainFrm.btnSelectFileClick(Sender: TObject);
begin
if odSelectFile.Execute then
   begin
     edFileName.Text:=odSelectFile.FileName;
   end;
end;

procedure TMainFrm.DataPackPacket(Sender: TObject; Data: Pointer; Size: Integer);
var
pB:PByte;

begin
//got a byte..
pB:=Data;
  if Size>0 then
     begin
      if pB^ = 6 then
        begin
         //ACK
         memDisplay.Lines.Insert(0,DateTimeToStr(Now)+': << ACK');
         tmrSend.Enabled:=true;
        end else
          if pB^ = 21 then
            begin
             //NAK
             tmrResend.Enabled:=true;
            end;
     end;
end;

end.
