$(function () {
  $('.controls').on('click', handleControlClick);
  $('input.volume').on('change', handleVolumeChange);
});

function handleControlClick(e) {
  var button = $(e.target).closest('button');

  if (button.length) {
    $.post('/control', JSON.stringify({'action': $(button).text()}), function (data) {
      console.log(data);
    }, 'json');
  }
}

function handleVolumeChange(e) {
  $.post('/control', JSON.stringify({'action': 'volume', 'slider': $(e.target).attr('name'), 'value': $(e.target).val()}), function (data) {
    console.log(data);
  }, 'json');
}
