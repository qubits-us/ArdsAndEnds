unit frmCamView;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, OverbyteIcsWndControl, OverbyteIcsWSocket, Vcl.StdCtrls, Vcl.ExtCtrls,VCL.Imaging.jpeg, Vcl.ExtDlgs,
  OverbyteIcsWSocketS,System.IOUtils,System.DateUtils;




type
     TPacketHeader = packed record
       PackIdent :array[0..1] of byte;
       Command   :byte;
       DataSize  :UINT32;
     end;

type
   TPacketSync = packed record
     Year :word;
     month:byte;
     day  :byte;
     hour :byte;
     min  :byte;
     sec  :byte;
   end;

type
  TPacketIdent = packed record
    VerLo :byte;
    VerHi :byte;
    CamNum:byte;
  end;



const
    MAX_IMSIZE =500000;
    IDENT_LO   =113;
    IDENT_HI   =126;

    CMD_NAK        =0;
    CMD_ACK        =1;
    CMD_SYNC       =2;
    CMD_IDENT      =3;
    CMD_JPG_BEGIN  =4;
    CMD_JPG_CHUNK  =5;
    CMD_JPG_END    =6;
    CMD_PAUSE      =7;
    CMD_CONFIG     =8;
    CMD_OTA_BEGIN  =9;
    CMD_OTA_CHUNK  =10;
    CMD_OTA_END    =11;

    JPG_CHUNK_SIZE  =1436;
    OTA_CHUNK_SIZE  =4096;



type
  TCamViewFrm = class(TForm)
    pnlTop: TPanel;
    pblBottom: TPanel;
    imgCam: TImage;
    btnClose: TButton;
    sckCam: TWSocket;
    mLog: TMemo;
    edBadPacks: TEdit;
    btnSave: TButton;
    dlgSavePic: TSavePictureDialog;
    chkDebug: TCheckBox;
    chkRecord: TCheckBox;
    tmrTimedOut: TTimer;
    procedure sckCamDataAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure sckCamSessionConnected(Sender: TObject; ErrCode: Word);
    procedure sckCamSessionClosed(Sender: TObject; ErrCode: Word);
    procedure sckCamError(Sender: TObject);
    procedure sckCamException(Sender: TObject; SocExcept: ESocketException);
    procedure sckCamSocksConnected(Sender: TObject; ErrCode: Word);
    procedure sckCamSocksError(Sender: TObject; Error: Integer; Msg: string);
    procedure sckCamBgException(Sender: TObject; E: Exception; var CanClose: Boolean);
    procedure btnCloseClick(Sender: TObject);
    procedure btnSaveClick(Sender: TObject);
    procedure chkDebugClick(Sender: TObject);
    procedure chkRecordClick(Sender: TObject);
    procedure StartRecording;
    procedure StopRecording;
    procedure SaveFrame;
    procedure piProcessIncoming;
    procedure piProcessAck;
    procedure piProcessNak;
    procedure piProcessIdent;
    procedure piProcessSync;
    procedure piProcessJpgBegin;
    procedure piProcessJpgChunk;
    procedure piProcessJpgEnd;
    procedure piProcessOtaChunk;
    procedure piSkip;
    procedure tmrTimedOutTimer(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  CamNum:integer;
  inBuff:TBytes;
  imgBuff :TBytes;
  imgSize:Int32;
  imgRecvd:integer;
  RecvCount:integer;
  RecvState:byte;
  haveRecvdImg:boolean;
  badPacks:integer;
  CamStrm:tMemoryStream;
  NumImgs:integer;
  haveImage:boolean;
  debugging:boolean;
  strmFile:TBufferedFileStream;
  vFolder:String;
  vFilePath:String;
  vFileName:String;
  recording:boolean;
  hdr:TPacketHeader;
  ident:TPacketIdent;
  sync:TPacketSync;
  end;

var
  CamViewFrm: TCamViewFrm;

implementation

{$R *.dfm}


procedure TCamViewFrm.FormCreate(Sender: TObject);
begin
 //
 vFilePath:=ExtractFilePath(Application.ExeName);
 vFilePath:=vFilePath+'video\';
 //one folder for each day..
 vFolder:=FormatDateTime('YYYY-MM-DD', now);

 SetLength(inBuff,MAX_IMSIZE);
 SetLength(imgBuff,MAX_IMSIZE);
 haveRecvdImg:=false;
 imgSize:=0;
 RecvCount:=0;
 RecvState := 0;
 CamStrm:=tMemoryStream.Create;
 mLog.Lines.Insert(0,'Ready..');
 sckCam.BufSize:=MAX_IMSIZE;
 NumImgs:=0;
 BadPacks:=0;
 haveImage:=false;
 debugging:=false;
 CamNum:=0;
 recording:=false;
end;


procedure TCamViewFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin

if recording then
 begin
  if Assigned(strmFile) then
  begin
   strmFile.FlushBuffer;
   strmFile.Free;
   strmFile := nil;
  end;
 end;


 //free cam stream
 try
 sckCam.Close;
 finally
   //
 end;

 //tell server form to free us..
 try
 PostMessage(TForm(Owner).Handle, WM_USER, 0, LongInt(Self));
 finally
   //
 end;
end;

procedure TCamViewFrm.btnCloseClick(Sender: TObject);
begin
Close;
end;

procedure TCamViewFrm.btnSaveClick(Sender: TObject);
var
aFileName:String;
begin
//save image..
if haveImage then
  begin
  if dlgSavePic.Execute then
    begin
      aFileName:=dlgSavePic.FileName;
      imgCam.Picture.SaveToFile(aFileName);
      ShowMessage('Image saved..');
    end;
  end;
end;

procedure TCamViewFrm.Button1Click(Sender: TObject);
begin
//stupid..
sckCamDataAvailable(nil, 0);

end;

procedure TCamViewFrm.chkDebugClick(Sender: TObject);
begin
if chkDebug.Checked then
  begin
    mLog.Height := 400;
    Height := Height + 320;
    debugging:=true;
  end else
    begin
    mLog.Height := 80;
    if (Height-320) < 400 then
     Height := 400 else
      Height := Height - 320;
    debugging:=false;
    end;
end;

procedure TCamViewFrm.chkRecordClick(Sender: TObject);
begin
//are we saving the stream..
if chkRecord.Checked then
  begin
  //start recording process..
   StartRecording;
  end else
    begin
    //stop recording..
    StopRecording;
    end;

end;

procedure TCamViewFrm.StartRecording;
var
newFolder:String;
begin
  //setup for recording the stream

 //check for day change..
 newFolder:=FormatDateTime('YYYY-MM-DD', now);
 if vFolder <> newFolder then vFolder:=newFolder;
 {$I-}
  MkDir(vFilePath+vFolder+'\');
{$I+}

 vFileName:=FormatDateTime('HH',now)+'.mjpeg';

 if not TFile.Exists(vFilePath+vFolder+'\'+vFileName) then
 begin
 strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmCreate);
 end else
   begin
   strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmOpenReadWrite);
   strmFile.Position:=strmFile.Size;
   end;


 recording :=true;
end;

procedure TCamViewFrm.SaveFrame;
var
newFileName:String;
newFolder:String;
begin
  //save current buffer contents..
  if Assigned(strmFile) then
    begin
    //check for hour change
     newFileName:=FormatDateTime('HH',now)+'.mjpeg';
     if newFileName <> vFileName then
       begin
       //check for day change..
         newFolder:=FormatDateTime('YYYY-MM-DD', now);
         if vFolder <> newFolder then vFolder:=newFolder;
          {$I-}
           MkDir(vFilePath+vFolder+'\');
          {$I+}

         vFileName := newFileName;
         strmFile.FlushBuffer;
         strmFile.Free;
          if not TFile.Exists(vFilePath+vFolder+'\'+vFileName) then
           begin
             strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmCreate);
             end else
              begin
               strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmOpenReadWrite);
               strmFile.Position:=strmFile.Size;
               end;
       end;
    strmFile.Position:=strmFile.Size;//seek the end..
    strmFile.WriteBuffer(imgSize,4);//write the frame size
    strmFile.WriteBuffer(imgBuff[0],imgSize);//then the frame..
    end;
end;

procedure TCamViewFrm.StopRecording;
begin
  //stop recording..
  recording := false;
  strmFile.FlushBuffer;
  strmFile.Free;
  strmFile := nil;

end;

procedure TCamViewFrm.tmrTimedOutTimer(Sender: TObject);
var
amnt:integer;
begin
//not getting this image..
tmrTimedOut.Enabled := false;
amnt:=sckCam.Receive(@inBuff[0],MAX_IMSIZE);
sckCam.Flush;


if debugging then
mLog.Lines.Insert(0,'!!Packet Timed Out!! tossing:'+intToStr(amnt));

RecvCount :=0;
RecvState :=0;
Inc(badPacks);
edBadPacks.Text:=IntToStr(badPacks);
//request next image..
hdr.PackIdent[0] := IDENT_LO;
hdr.PackIdent[1] := IDENT_HI;
hdr.Command:=CMD_JPG_BEGIN;
hdr.DataSize :=0;
sckCam.Send(@hdr,sizeof(TPacketHeader));
end;


procedure TCamViewFrm.sckCamBgException(Sender: TObject; E: Exception; var CanClose: Boolean);
begin
mlog.Lines.Insert(0,E.Message);
end;

procedure TCamViewFrm.sckCamDataAvailable(Sender: TObject; ErrCode: Word);
var
Len:integer;
begin
 if debugging then
   begin
   //don't blow up..
   if mLog.Lines.Count > 999 then mLog.Lines.Clear;
   mLog.Lines.Insert(0,'*** Data Avail Error Code:'+IntToStr(ErrCode)+' Current Recv Count:'+IntToStr(RecvCount)+' ****');
   end;

if RecvState = 0 then
  begin
  //recv header..
  Len:=sckCam.Receive(@inBuff[RecvCount],SizeOf(TPacketHeader)-RecvCount);
  if debugging then
    mLog.Lines.Insert(0,'Header Bytes Recvd:'+IntToStr(Len));
  if Len < 0 then Len :=0;
  //got something..
  if Len > 0 then
   begin
   tmrTimedOut.Enabled := false;
     RecvCount := RecvCount + Len;
   if RecvCount = SizeOf(TPacketHeader) then
     begin
       move(inBuff[0],hdr,SizeOf(TPacketHeader));
       RecvCount := 0;
       if (hdr.PackIdent[0] = IDENT_LO) and (hdr.PackIdent[1] = IDENT_HI) then
         begin
         //valid ident..
         if hdr.DataSize > 0  then
           begin
             if hdr.DataSize > MAX_IMSIZE then
               begin
               if debugging then
                mLog.Lines.Insert(0,'Too Big:'+IntToStr(hdr.DataSize));
                 hdr.DataSize :=0;
                 RecvCount := 0;
                 sckCam.Flush;
                 exit;
               end;
             RecvCount := sckCam.Receive(@inBuff[RecvCount],hdr.DataSize);
            if RecvCount < 0 then RecvCount :=0;//zero it on error..
            if debugging then
                mLog.Lines.Insert(0,'Recvd Extra:'+IntToStr(RecvCount)+' of '+intToStr(hdr.DataSize));
             if RecvCount = hdr.DataSize then
               begin
               //got it..
               piProcessIncoming;
               //get ready for next..
               RecvCount :=0;
               RecvState :=0;
               end else
                  begin
                   RecvState := 1;//receiving more later..
                   tmrTimedOut.Enabled:=true;
                   end;
           end else
             begin
               //just a header..
               //process
               piProcessIncoming;
               //get ready for next
               RecvCount :=0;
               RecvState :=0;
             end;



         end else
           begin
             //bad ident..
                Len := sckCam.Receive(@inBuff[0],MAX_IMSIZE);
            if debugging then
                mLog.Lines.Insert(0,'Error: Bad Ident: Tossing:'+IntToStr(Len));
                sckCam.Flush;
             tmrTimedOut.Enabled := true;
           end;
     end;//we got a packet..
   end;//if we have something
  end else
    begin
     if debugging then
      mLog.Lines.Insert(0,'Remaining:'+IntToStr(hdr.DataSize - RecvCount)+' of '+intToStr(hdr.DataSize));
     Len := sckCam.Receive(@inBuff[RecvCount],hdr.DataSize-RecvCount);
     if (JPG_CHUNK_SIZE < (hdr.DataSize - RecvCount)) then
       if (Len < JPG_CHUNK_SIZE) and (Len > 0) then
         begin
          mLog.Lines.Insert(0,'**** Missing Packets ******');
         end;


     if debugging then
     mLog.Lines.Insert(0,'Recvd Chunk:'+IntToStr(Len)+' Remaining:'+intToStr(hdr.DataSize-(RecvCount+Len)));
     if Len < 0 then Len :=0;//zero on error..
     RecvCount := RecvCount + Len;
     if debugging then
     mLog.Lines.Insert(0,'RecvCount:'+IntToStr(RecvCount)+' hdr.DataSize:'+intToStr(hdr.DataSize));
     tmrTimedOut.Enabled := false;
      if RecvCount = hdr.DataSize then
      begin
        tmrTimedOut.Enabled:=false;
       //got it..
       piProcessIncoming;
       //get ready for next..
       RecvCount :=0;
       RecvState :=0;
      end else tmrTimedOut.Enabled:=true;
    end;
end;


procedure TCamViewFrm.piProcessIncoming;
begin
  //process incoming packet..
  if debugging then
   mLog.Lines.Insert(0,'Processing command:'+IntToStr(hdr.Command));

  case hdr.Command of
  CMD_NAK:      piProcessNak; //didn't like something..
  CMD_ACK:      piProcessAck; //acknowledge
  CMD_IDENT:    piProcessIdent; //identify
  CMD_SYNC:     piProcessSync; //sync time
  CMD_JPG_BEGIN:piProcessJpgBegin;  //getting a jpg..
  CMD_JPG_CHUNK:piProcessJpgChunk;  //getting a jpg chunk..
  CMD_JPG_END:  piProcessJpgEnd;  //got a jpg..
  CMD_OTA_CHUNK:piProcessOtaChunk ; //send a chunk..
  end;
end;

procedure TCamViewFrm.piProcessAck;
begin
  //ack..
end;

procedure TCamViewFrm.piProcessNak;
begin
  //not acknowleged..

end;

procedure TCamViewFrm.piProcessIdent;
var
outb:tBytes;
begin
  //identify
  if hdr.DataSize = SizeOf(TPacketIdent) then
   begin
   if debugging then
    mLog.Lines.Insert(0,'Processing Ident..');
    move(inBuff[0],ident,SizeOf(TPacketIdent));
    mLog.Lines.Insert(0,'Identified as Cam :'+IntToStr(ident.CamNum));
    mLog.Lines.Insert(0,'Firmware ver: '+IntToStr(ident.VerHi)+'.'+IntToStr(ident.VerLo));;
    ident.CamNum := CamNum;
    mLog.Lines.Insert(0,'Setting cam to :'+IntToStr(ident.CamNum));
    setLength(outb,sizeof(TPacketHeader)+sizeof(TPacketIdent));
    move(hdr,outb[0],sizeof(TPacketHeader));
    move(ident,outb[sizeof(TPacketHeader)],sizeof(TPacketIdent));
    sckCam.Send(outb,Length(outb));
    setLength(outb,0);
   end;

end;

procedure TCamViewFrm.piProcessSync;
var
year, month, day, hour, minute, second, millisecond: Word;
outb:tBytes;
amnt:integer;
begin
  //sync time
  if debugging then
   mLog.Lines.Insert(0,'Processing Sync..');
   DecodeDateTime(now,year,month,day,hour,minute,second,millisecond);
   sync.Year  := Year;
   sync.day   := Day;
   sync.month := Month;
   sync.hour  := Hour;
   sync.min   := Minute;
   sync.sec   := 0;
  if debugging then
   mLog.Lines.Insert(0,'Sync Date Time: '+IntToStr(sync.Year)+'/'+IntToStr(sync.month)+
    '/'+IntToStr(sync.day)+'  '+IntToStr(sync.hour)+':'+IntToStr(sync.min));

  hdr.Command := CMD_SYNC;
  hdr.DataSize := SizeOf(TPacketSync);
  setLength(outb,sizeof(TPacketHeader)+sizeof(TPacketSync));
   move(hdr,outb[0],sizeof(TPacketHeader));
   move(sync,outb[sizeof(TPacketHeader)],sizeof(TPacketSync));
   sckCam.Send(outb,Length(outb));
   setLength(outb,0);
//  sckCam.Send(@hdr,sizeof(TPacketHeader));
  //sckCam.Send(@sync,SizeOf(TPacketSync));
end;

procedure TCamViewFrm.piProcessOtaChunk;
begin
  //ota chunk..
end;

procedure TCamViewFrm.piSkip;
begin
  //skipping..
  //get ready for next..
  RecvCount :=0;
  RecvState :=0;
  //ask for next frame..
  hdr.Command := CMD_JPG_BEGIN;
  hdr.DataSize := 0;
  sckCam.Send(@hdr,sizeof(TPacketHeader));
end;


procedure TCamViewFrm.piProcessJpgBegin;
begin
  //start a new jpg..
  if hdr.DataSize = sizeOf(imgSize) then
    move(inBuff[0],imgSize,SizeOf(imgSize));
  if debugging then
    mLog.Lines.Insert(0,'Begin image: '+IntToStr(imgSize));
  imgRecvd :=0;
 //request chunk..
 hdr.Command := CMD_JPG_CHUNK;
 hdr.DataSize := 0;
 sckCam.Send(@hdr,sizeof(TPacketHeader));
end;

procedure TCamViewFrm.piProcessJpgChunk;
var
sent,dsize:integer;
begin
  //save a jpg chunk..
 move(inBuff[0],imgBuff[imgRecvd],hdr.DataSize);
 imgRecvd:=imgRecvd+hdr.DataSize;
 //request another..
 hdr.Command := CMD_JPG_CHUNK;
 dsize :=hdr.DataSize;
 hdr.DataSize := 0;
 sent := sckCam.Send(@hdr,sizeof(TPacketHeader));
  if debugging then
    mLog.Lines.Insert(0,'chunk recvd: '+IntToStr(dsize)+' total recvd:'+IntToStr(imgRecvd)+' remaining:'+IntToStr(imgSize-imgRecvd)+' sent:'+IntToStr(sent));
end;



procedure TCamViewFrm.piProcessJpgEnd;
var
jpg:tJpegImage;
begin
 jpg :=nil;
 //move in last chunk..
  move(inBuff[0],imgBuff[imgRecvd],hdr.DataSize);
  imgRecvd:=imgRecvd+hdr.DataSize;

  //got a image..
       if debugging then
          mLog.Lines.Insert(0,'Recieved image:'+IntToStr(imgSize)+' byte recvd:'+IntToStr(imgRecvd));

         CamStrm.SetSize(imgSize);
         CamStrm.Position:=0;
         CamStrm.Write(imgBuff[0],imgSize);
         CamStrm.Position:=0;
           try
             jpg:=tJpegImage.Create;
             jpg.LoadFromStream(CamStrm);
             imgCam.Picture.Assign(jpg);
             haveImage:=true;
             if recording then SaveFrame;
           except on e:exception do
             begin
              mLog.Lines.Insert(0,e.Message);
              Inc(BadPacks);
              edBadPacks.Text:=IntToStr(BadPacks);
              imgSize:=0;
              RecvCount:=0;
             end;
           end;

           if Assigned(jpg) then jpg.Free;

             imgSize:=0;
   //ask for next frame..
   hdr.Command := CMD_JPG_BEGIN;
   hdr.DataSize := 0;
   sckCam.Send(@hdr,sizeof(TPacketHeader));
end;




{
procedure TCamViewFrm.sckCamDataAvailable(Sender: TObject; ErrCode: Word);
var
Len:integer;
jpg:tJpegImage;
begin
//got something..
 jpg:=nil;
 if imgSize=0 then
    begin
     Len:=sckCam.Receive(@imgBuff[0],SizeOf(imgSize));
      if Len>=sizeOf(imgSize) then
       move(imgBuff[0],imgSize,SizeOf(imgSize));
       RecvCount:=0;
       if imgSize> MAX_IMSIZE then
         begin
     //    if debugging then
       //   mLog.Lines.Insert(0,'Image too big:'+IntToStr(imgSize));
          imgSize:=0;//throw it away..
         end else
       if debugging then
       mLog.Lines.Insert(0,'Recieving image:'+IntToStr(imgSize));
    end
    else
   begin

   if imgSize>0 then
    begin
      // if debugging then
      //    mLog.Lines.Insert(0,'Requesting: '+IntToStr(imgSize-RecvCount));

     Len:=sckCam.Receive(@imgBuff[RecvCount],imgSize-RecvCount);
     //  if debugging then
     //     mLog.Lines.Insert(0,'Recieved: '+IntToStr(Len));

    if Len>0 then
     begin
     RecvCount:=Len+RecvCount;
       if debugging then
       mLog.Lines.Insert(0,'Recieved image chunk:'+IntToStr(Len));

      if (RecvCount = imgSize) then
        begin
       if debugging then
          mLog.Lines.Insert(0,'Recieved image:'+IntToStr(imgSize));
         CamStrm.SetSize(imgSize);
         CamStrm.Position:=0;
         CamStrm.Write(imgBuff[0],imgSize);
         CamStrm.Position:=0;
           try
             jpg:=tJpegImage.Create;
             jpg.LoadFromStream(CamStrm);
             imgCam.Picture.Assign(jpg);
             haveImage:=true;
             if recording then SaveFrame;
           except on e:exception do
             begin
              mLog.Lines.Insert(0,e.Message);
              Inc(BadPacks);
              edBadPacks.Text:=IntToStr(BadPacks);
              imgSize:=0;
              RecvCount:=0;
             end;
           end;

           if Assigned(jpg) then jpg.Free;
             imgSize:=0;
             RecvCount:=0;
        end;
     end else
       begin
       //if debugging then
         // mLog.Lines.Insert(0,'dropping : len less than 0: '+IntToStr(Len));
       // imgSize :=0;
      //  RecvCount :=0;

       end;
    end else
     begin
       if debugging then
          mLog.Lines.Insert(0,'dropping : image size less than 0: '+IntToStr(imgSize));
     imgSize :=0;
     RecvCount :=0;
     end;
      end;
end;   }

procedure TCamViewFrm.sckCamError(Sender: TObject);
begin
mLog.Lines.Insert(0,'Error');
end;

procedure TCamViewFrm.sckCamException(Sender: TObject; SocExcept: ESocketException);
begin
mLog.Lines.Insert(0,'Sock Error:')
end;

procedure TCamViewFrm.sckCamSessionClosed(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Disconnected..');
Close;
end;

procedure TCamViewFrm.sckCamSessionConnected(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Connected..');
end;

procedure TCamViewFrm.sckCamSocksConnected(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Socks Connected:'+IntToStr(ErrCode));
end;

procedure TCamViewFrm.sckCamSocksError(Sender: TObject; Error: Integer; Msg: string);
begin
mLog.Lines.Insert(0,'Socks Error:'+IntToStr(Error)+' '+Msg);
end;

end.
