object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'AckMe'
  ClientHeight = 463
  ClientWidth = 359
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDesktopCenter
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 359
    Height = 145
    Align = alTop
    TabOrder = 0
    ExplicitWidth = 402
    object lblPortNum: TLabel
      Left = 16
      Top = 8
      Width = 69
      Height = 15
      Caption = 'Port Number'
    end
    object lblFileName: TLabel
      Left = 16
      Top = 53
      Width = 75
      Height = 15
      Caption = 'File to transfer'
    end
    object lblBaud: TLabel
      Left = 127
      Top = 8
      Width = 53
      Height = 15
      Caption = 'Baud Rate'
    end
    object lblDelim: TLabel
      Left = 280
      Top = 8
      Width = 48
      Height = 15
      Caption = 'Delimiter'
    end
    object btnBegin: TButton
      Left = 280
      Top = 104
      Width = 60
      Height = 25
      Caption = 'Begin'
      TabOrder = 0
      OnClick = btnBeginClick
    end
    object btnSelectFile: TButton
      Left = 280
      Top = 69
      Width = 60
      Height = 25
      Caption = 'Select'
      TabOrder = 1
      OnClick = btnSelectFileClick
    end
    object edPort: TEdit
      Left = 16
      Top = 24
      Width = 105
      Height = 23
      TabOrder = 2
      Text = '3'
    end
    object edFileName: TEdit
      Left = 16
      Top = 70
      Width = 258
      Height = 23
      TabOrder = 3
    end
    object cbBaud: TComboBox
      Left = 127
      Top = 24
      Width = 145
      Height = 23
      ItemIndex = 6
      TabOrder = 4
      Text = '9600'
      Items.Strings = (
        '150'
        '300'
        '600'
        '1200'
        '2400'
        '4800'
        '9600'
        '19200'
        '38400'
        '57600'
        '115200')
    end
    object pbProgress: TProgressBar
      Left = 16
      Top = 112
      Width = 258
      Height = 17
      TabOrder = 5
    end
    object edDelim: TEdit
      Left = 280
      Top = 24
      Width = 57
      Height = 23
      Alignment = taCenter
      MaxLength = 1
      TabOrder = 6
      Text = ';'
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 145
    Width = 359
    Height = 318
    Align = alClient
    TabOrder = 1
    ExplicitWidth = 402
    object memDisplay: TMemo
      Left = 1
      Top = 1
      Width = 357
      Height = 316
      Align = alClient
      Color = clBlack
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clAqua
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      StyleElements = [seBorder]
      ExplicitWidth = 400
    end
  end
  object ComPort: TApdComPort
    TraceName = 'APRO.TRC'
    LogName = 'APRO.LOG'
    Left = 176
    Top = 184
  end
  object odSelectFile: TOpenDialog
    Left = 32
    Top = 192
  end
  object DataPack: TApdDataPacket
    Enabled = False
    StartCond = scAnyData
    EndCond = [ecPacketSize]
    ComPort = ComPort
    PacketSize = 1
    OnPacket = DataPackPacket
    Left = 160
    Top = 312
  end
  object tmrSend: TTimer
    Enabled = False
    Interval = 1
    OnTimer = tmrSendTimer
    Left = 88
    Top = 312
  end
  object tmrResend: TTimer
    Enabled = False
    Interval = 1
    OnTimer = tmrResendTimer
    Left = 112
    Top = 216
  end
  object tmrBegin: TTimer
    Enabled = False
    Interval = 2000
    OnTimer = tmrBeginTimer
    Left = 240
    Top = 265
  end
end
