var start = new Date();
$(document).ready(function() {
  $("table.tablesorter").each(function() {
    if($(this).hasClass("reverse")) {
      $(this).tablesorter({sortList:[[0,1]]});
    } else {
      $(this).tablesorter({sortList:[[0,0]]});
    }
  });
  $("a[rel='ext']").each(function() {
    $(this).attr("target", "_blank");
  });
  $("p.guide a").each(function(i) {
    var imgsrc = $(this).children("img").attr("src");
    $(this).bind("click", function(evt) {
      var img = $("#imgfocus");
      if(img.attr("src") == imgsrc && !img.hasClass("hide")) {
        img.addClass("hide");
        return false;
      }
      img.attr("src", imgsrc);
      if(img.hasClass("hide")) {
        img.removeClass("hide");
        $("body").animate({scrollTop:$("body").height()}, 1000);
      }
      return false;
    });
  });
  $("#showmore").bind("click", function(evt) {
    var e = $("tr.morehidden");
    if(e.css("display") == "none")
      e.show();
    else
      e.hide();
  });
  $("#wrap").fadeIn(150);
  $("a.nojs").removeClass("nojs");
  $("#jsload").html(" <em>+"+(new Date()-start)+"ms</em>");
  reloadPage = function() {
    window.location.replace(""+window.location);
  }
  nextRound = function() {
    $.ajax({
      url: "./?archive&winner",
      success: reloadPage,
      error: reloadPage
    });
  }
});
$(window).load(function() {
  $("#evtwinner").animate({opacity: "show", height: "show"}, 3500);
  var time = 0;
  try {
    time = parseInt($("#evtwinner span.cd").html());
  } catch(ex) {}
  if(time > 0) {
    cdTick = function() {
      if($("#evtwinner").css("display") != "block")
        return;
      try {
        var timeleft = parseInt($("#evtwinner span.cd").html());
        if(timeleft < 1)
          $("#evtwinner").animate({opacity: "hide", height: "hide"}, 1500);
        else
          $("#evtwinner span.cd").html(timeleft-1);
      } catch(ex) {
        $("#evtwinner span.gray").fadeOut(300);
      }
    }
    setInterval('cdTick();', 1000);
  } else
    setTimeout('$("#evtwinner").animate({opacity: "hide", height: "hide"}, 1500);', 15000);
});
