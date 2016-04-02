#ifndef __TOKENDEFINITIONS_H
#define __TOKENDEFINITIONS_H

enum class eViews {
    rootView,
    detailView
};

enum class eViewElementsDetail {
    background,
    footer,
    info,
    message,
    scrollbar,
    watch
};

enum class eViewElementsRoot {
    background,
    header,
    footer,
    infopane,
    watch,
    message,
    scrollbar
};

enum class eViewGrids {
    cover,
    detail,
    list,
};

enum class eViewDetailViewGrids {
    extras
};

enum class eTokenGridInt {
    viewmode = 0,
    viewgroup,
    viewCount,
    viewoffset,
    viewoffsetpercent,
    duration,
    year,
    hasthumb,
    hasart,
    ismovie,
    isepisode,
    isdirectory,
    isshow,
    isseason,
    isclip,
    originallyAvailableYear,
    originallyAvailableMonth,
    originallyAvailableDay,
    season,
    episode,
    leafCount,
    viewedLeafCount,
    childCount,
    rating,
    hasseriesthumb,
    hasbanner,
    columns,
    rows,
    position,
    totalcount,
    bitrate,
    width,
    height,
    audioChannels,
    isdummy,
    isserver,
    serverport,
    extratype
};

enum class eTokenGridStr {
    title = 0,
    orginaltitle,
    summary,
    tagline,
    contentrating,
    ratingstring,
    studio,
    thumb,
    art,
    seriestitle,
    seriesthumb,
    banner,
    videoResolution,
    aspectRatio,
    audioCodec,
    videoCodec,
    container,
    videoFrameRate,
    serverstartpointname,
    serverip,
    serverversion,
    tabname
};

enum class eTokenGridActorLst {
    roles = 0
};

enum class eTokenGridGenresLst {
    genres = 0
};

enum class eTokenMessageInt {
    displaymessage = 0
};

enum class eTokenMessageStr {
    message = 0
};

enum class eTokenScrollbarInt {
    height = 0,
    offset,
    hasscrollbar
};

enum class eTokenBackgroundInt {
    viewmode = 0,
    isdirectory
};

enum class eTokenBackgroundStr {
    selecteditembackground = 0,
    currentdirectorybackground
};

enum class eTokenDetailBackgroundInt {
    hasfanart = 0,
    hascover
};

enum class eTokenDetailBackgroundStr {
    fanartpath = 0,
    coverpath
};

enum class eTokenTimeInt {
    sec = 0,
    min,
    hour,
    hmins,
    day,
    year
};

enum class eTokenTimeStr {
    time = 0,
    dayname,
    daynameshort,
    dayleadingzero,
    month,
    monthname,
    monthnameshort
};

enum class eTokenFooterInt {
    red1 = 0,
    red2,
    red3,
    red4,
    green1,
    green2,
    green3,
    green4,
    yellow1,
    yellow2,
    yellow3,
    yellow4,
    blue1,
    blue2,
    blue3,
    blue4
};

enum class eTokenFooterStr {
    red = 0,
    green,
    yellow,
    blue
};

enum class eTokenProgressbarInt {

};

enum class eTokenProgressbarStr {

};

/*<TranscodeSession key="jhul52ltx5lpiudi" 
 * throttled="0" 
 * progress="2.9000000953674316" 
 * speed="3.2999999523162842" 
 * duration="8886000" 
 * remaining="2677" 
 * context="streaming" 
 * videoDecision="copy" 
 * audioDecision="transcode" 
 * subtitleDecision="transcode" 
 * protocol="http" 
 * container="mkv" 
 * videoCodec="h264" 
 * audioCodec="aac" 
 * audioChannels="2" 
 * width="1920" 
 * height="808" />
 */
enum class eTokenTranscodeinfoStr {

};

#endif //__TOKENDEFINITIONS_H