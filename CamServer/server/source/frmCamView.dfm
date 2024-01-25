object CamViewFrm: TCamViewFrm
  Left = 0
  Top = 0
  BorderStyle = bsSizeToolWin
  Caption = 'ESP Cam :'
  ClientHeight = 717
  ClientWidth = 934
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDefault
  ShowInTaskBar = True
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 15
  object imgCam: TImage
    Left = 0
    Top = 41
    Width = 384
    Height = 555
    Align = alClient
    Stretch = True
    OnClick = imgCamClick
    ExplicitLeft = 4
    ExplicitTop = 39
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 934
    Height = 41
    Align = alTop
    TabOrder = 0
    Visible = False
    DesignSize = (
      934
      41)
    object lblRecvInfo: TLabel
      Left = 6
      Top = 4
      Width = 91
      Height = 15
      Caption = 'Waiting for data..'
    end
    object lblRecordInfo: TLabel
      Left = 6
      Top = 22
      Width = 78
      Height = 15
      Caption = 'Recording off..'
    end
    object lblRejected: TLabel
      Left = 816
      Top = 15
      Width = 20
      Height = 15
      Caption = 'Bad'
    end
    object edBadPacks: TEdit
      Left = 845
      Top = 12
      Width = 74
      Height = 23
      Anchors = [akTop, akRight]
      ReadOnly = True
      TabOrder = 0
      Text = '0'
    end
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 596
    Width = 934
    Height = 41
    Align = alBottom
    TabOrder = 1
    Visible = False
    DesignSize = (
      934
      41)
    object btnConfig: TButton
      Left = 844
      Top = 8
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Config'
      TabOrder = 0
      OnClick = btnConfigClick
    end
    object btnSave: TButton
      Left = 10
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Snapshot'
      TabOrder = 1
      OnClick = btnSaveClick
    end
    object chkDebug: TCheckBox
      Left = 97
      Top = 4
      Width = 97
      Height = 17
      Caption = 'Show Debug'
      TabOrder = 2
      OnClick = chkDebugClick
    end
    object chkRecord: TCheckBox
      Left = 200
      Top = 4
      Width = 97
      Height = 17
      Caption = 'Record'
      Enabled = False
      TabOrder = 3
      OnClick = chkRecordClick
    end
    object cbSilenceAlert: TCheckBox
      Left = 96
      Top = 22
      Width = 97
      Height = 17
      Caption = 'Silence Alert'
      TabOrder = 4
      OnClick = cbSilenceAlertClick
    end
    object cbRemoteRecord: TCheckBox
      Left = 200
      Top = 22
      Width = 122
      Height = 17
      Caption = 'Remote Rec Ctrl'
      TabOrder = 5
    end
  end
  object mLog: TMemo
    Left = 0
    Top = 637
    Width = 934
    Height = 80
    Align = alBottom
    Color = clBlack
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clAqua
    Font.Height = -12
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 2
    Visible = False
    WordWrap = False
    StyleElements = [seBorder]
  end
  object pnlConfig: TPanel
    Left = 384
    Top = 41
    Width = 550
    Height = 555
    Align = alRight
    TabOrder = 3
    Visible = False
    object lblRes: TLabel
      Left = 10
      Top = 19
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Resolution'
    end
    object lblQuality: TLabel
      Left = 10
      Top = 50
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Quality'
    end
    object lblBrightness: TLabel
      Left = 10
      Top = 77
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Brightness'
    end
    object lblContrast: TLabel
      Left = 10
      Top = 117
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Contrast'
    end
    object lblSaturation: TLabel
      Left = 10
      Top = 160
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Saturation'
    end
    object lblEffect: TLabel
      Left = 10
      Top = 199
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Effect'
    end
    object lblAWB: TLabel
      Left = 10
      Top = 230
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AWB'
    end
    object lblAWBgain: TLabel
      Left = 10
      Top = 253
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AWB Gain'
    end
    object lblWBmode: TLabel
      Left = 10
      Top = 282
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'WB Mode'
    end
    object lblAecSensor: TLabel
      Left = 10
      Top = 312
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AEC Sensor'
    end
    object lblAecDsp: TLabel
      Left = 10
      Top = 344
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AEC DSP'
    end
    object lblAElevel: TLabel
      Left = 10
      Top = 379
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AE Level'
    end
    object lblAGC: TLabel
      Left = 10
      Top = 452
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'AGC'
    end
    object lblBPC: TLabel
      Left = 260
      Top = 32
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'BPC'
    end
    object lblWpc: TLabel
      Left = 260
      Top = 60
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'WPC'
    end
    object lblRawGMA: TLabel
      Left = 260
      Top = 90
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Raw GMA'
    end
    object lblLensCorr: TLabel
      Left = 260
      Top = 120
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Lens Correction'
    end
    object lblHmirror: TLabel
      Left = 260
      Top = 150
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'H-Mirror'
    end
    object lblVFlip: TLabel
      Left = 260
      Top = 180
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'V-Flip'
    end
    object lblDCW: TLabel
      Left = 260
      Top = 210
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'DCW (Downsize Enable)'
    end
    object lblColorBar: TLabel
      Left = 260
      Top = 240
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Color Bar'
    end
    object lblGainCeiling: TLabel
      Left = 10
      Top = 485
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Gain Ceiling'
    end
    object lblExposure: TLabel
      Left = 10
      Top = 415
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Exposure'
    end
    object lblGain: TLabel
      Left = 10
      Top = 519
      Width = 80
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Gain'
    end
    object lblDebug: TLabel
      Left = 260
      Top = 272
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Debug'
    end
    object lblCapDelay: TLabel
      Left = 260
      Top = 300
      Width = 150
      Height = 15
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'Capture Interval'
    end
    object cmbRes: TComboBox
      Left = 104
      Top = 16
      Width = 145
      Height = 23
      Style = csDropDownList
      ItemIndex = 6
      TabOrder = 0
      Text = 'CIF    -400x296'
      Items.Strings = (
        '96X96    -96x96'
        'QQVGA    -160x120'
        'QCIF    -176x144'
        'HQVGA    -240x176'
        '240X240    -240x240'
        'QVGA    -320x240'
        'CIF    -400x296'
        'HVGA    -480x320'
        'VGA    -640x480'
        'SVGA    -800x600'
        'XGA    -1024x768'
        'HD    -1280x720'
        'SXGA    -1280x1024'
        'UXGA    -1600x1200')
    end
    object tbQuality: TTrackBar
      Left = 104
      Top = 45
      Width = 150
      Height = 20
      Max = 63
      Min = 10
      Position = 10
      TabOrder = 1
      TickStyle = tsNone
    end
    object tbBrightness: TTrackBar
      Left = 104
      Top = 77
      Width = 150
      Height = 30
      Max = 2
      Min = -2
      TabOrder = 2
    end
    object tbContrast: TTrackBar
      Left = 104
      Top = 117
      Width = 150
      Height = 30
      Max = 2
      Min = -2
      TabOrder = 3
    end
    object cmbEffect: TComboBox
      Left = 104
      Top = 196
      Width = 145
      Height = 23
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 4
      Text = 'No Effect'
      Items.Strings = (
        'No Effect'
        'Negative'
        'Grayscale'
        'Red Tint'
        'Green Tint'
        'Blue Tint'
        'Sepia')
    end
    object tsAWB: TToggleSwitch
      Left = 104
      Top = 225
      Width = 73
      Height = 20
      TabOrder = 5
    end
    object tsAWBgain: TToggleSwitch
      Left = 104
      Top = 251
      Width = 73
      Height = 20
      TabOrder = 6
    end
    object cmbWBmode: TComboBox
      Left = 104
      Top = 280
      Width = 145
      Height = 23
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 7
      Text = 'Auto'
      Items.Strings = (
        'Auto'
        'Sunny'
        'Cloudy'
        'Office'
        'Home')
    end
    object tsAECsensor: TToggleSwitch
      Left = 104
      Top = 309
      Width = 73
      Height = 20
      TabOrder = 8
      OnClick = tsAECsensorClick
    end
    object tsAECDSP: TToggleSwitch
      Left = 104
      Top = 341
      Width = 73
      Height = 20
      TabOrder = 9
    end
    object tbAElevel: TTrackBar
      Left = 104
      Top = 373
      Width = 150
      Height = 45
      Max = 2
      Min = -2
      TabOrder = 10
    end
    object tsAGC: TToggleSwitch
      Left = 104
      Top = 449
      Width = 73
      Height = 20
      TabOrder = 11
      OnClick = tsAGCClick
    end
    object tbGainCeiling: TTrackBar
      Left = 104
      Top = 481
      Width = 150
      Height = 30
      Max = 128
      Min = 2
      Position = 2
      TabOrder = 12
      TickStyle = tsNone
    end
    object tsColorBar: TToggleSwitch
      Left = 432
      Top = 240
      Width = 73
      Height = 20
      TabOrder = 13
    end
    object tsDCW: TToggleSwitch
      Left = 432
      Top = 210
      Width = 73
      Height = 20
      TabOrder = 14
    end
    object tsVFlip: TToggleSwitch
      Left = 432
      Top = 180
      Width = 73
      Height = 20
      TabOrder = 15
    end
    object tsHmirror: TToggleSwitch
      Left = 432
      Top = 150
      Width = 73
      Height = 20
      TabOrder = 16
    end
    object tsLensCorr: TToggleSwitch
      Left = 432
      Top = 120
      Width = 73
      Height = 20
      TabOrder = 17
    end
    object tsRawGMA: TToggleSwitch
      Left = 432
      Top = 90
      Width = 73
      Height = 20
      TabOrder = 18
    end
    object tsWPC: TToggleSwitch
      Left = 432
      Top = 60
      Width = 73
      Height = 20
      TabOrder = 19
    end
    object tsBPC: TToggleSwitch
      Left = 432
      Top = 32
      Width = 73
      Height = 20
      TabOrder = 20
    end
    object tbSaturation: TTrackBar
      Left = 104
      Top = 157
      Width = 150
      Height = 30
      Max = 2
      Min = -2
      TabOrder = 21
    end
    object tbExposure: TTrackBar
      Left = 104
      Top = 413
      Width = 150
      Height = 30
      Max = 1200
      TabOrder = 22
      TickStyle = tsNone
    end
    object tbGain: TTrackBar
      Left = 104
      Top = 517
      Width = 150
      Height = 30
      Max = 30
      TabOrder = 23
      TickStyle = tsNone
    end
    object btnGetConfig: TButton
      Left = 360
      Top = 344
      Width = 75
      Height = 25
      Caption = 'Get'
      TabOrder = 24
      OnClick = btnGetConfigClick
    end
    object btnPutConfig: TButton
      Left = 441
      Top = 344
      Width = 75
      Height = 25
      Caption = 'Put'
      Enabled = False
      TabOrder = 25
      OnClick = btnPutConfigClick
    end
    object GroupBox1: TGroupBox
      Left = 288
      Top = 392
      Width = 233
      Height = 145
      Caption = 'Recording Options'
      TabOrder = 26
      object lblRecMode: TLabel
        Left = 16
        Top = 32
        Width = 50
        Height = 15
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Mode'
      end
      object lblRecInterval: TLabel
        Left = 16
        Top = 60
        Width = 50
        Height = 15
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Interval'
      end
      object cmbRecordMode: TComboBox
        Left = 72
        Top = 29
        Width = 145
        Height = 23
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 0
        Text = 'Full Frame Rate'
        Items.Strings = (
          'Full Frame Rate'
          'TimerInterval')
      end
      object edRecInterval: TEdit
        Left = 72
        Top = 58
        Width = 145
        Height = 23
        NumbersOnly = True
        TabOrder = 1
        Text = '60000'
      end
      object btnRecApply: TButton
        Left = 144
        Top = 95
        Width = 75
        Height = 25
        Caption = 'Apply'
        TabOrder = 2
        OnClick = btnRecApplyClick
      end
    end
    object btnPreview: TButton
      Left = 279
      Top = 344
      Width = 75
      Height = 25
      Caption = 'Preview'
      Enabled = False
      TabOrder = 27
      OnClick = btnPreviewClick
    end
    object tsDebug: TToggleSwitch
      Left = 432
      Top = 270
      Width = 73
      Height = 20
      TabOrder = 28
    end
    object edCapDelay: TEdit
      Left = 416
      Top = 296
      Width = 84
      Height = 23
      NumbersOnly = True
      TabOrder = 29
      Text = '100'
    end
  end
  object sckCam: TWSocket
    LineEnd = #13#10
    Port = '12001'
    Proto = 'tcp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    LocalPort = '0'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = [wsoTcpNoDelay]
    ListenBacklog = 15
    OnDataAvailable = sckCamDataAvailable
    OnSessionClosed = sckCamSessionClosed
    OnSessionConnected = sckCamSessionConnected
    OnSocksConnected = sckCamSocksConnected
    OnError = sckCamError
    OnBgException = sckCamBgException
    OnSocksError = sckCamSocksError
    SocketErrs = wsErrTech
    onException = sckCamException
    Left = 312
    Top = 56
  end
  object dlgSavePic: TSavePictureDialog
    Title = 'Same Pic'
    Left = 272
    Top = 64
  end
  object tmrTimedOut: TTimer
    Enabled = False
    Interval = 60000
    OnTimer = tmrTimedOutTimer
    Left = 352
    Top = 56
  end
  object tmrAlert: TTimer
    Enabled = False
    Interval = 3000
    OnTimer = tmrAlertTimer
    Left = 296
    Top = 112
  end
  object tmrRecord: TTimer
    Enabled = False
    OnTimer = tmrRecordTimer
    Left = 312
    Top = 168
  end
end
