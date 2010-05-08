<?php
// $Id$

/**
 * @file
 * Administrative functions for the OAuth Connector module.
 *
 * This provides the UI to list, create, edit and delete providers.
 */

/**
 * Output a list of providers.
 */
function oauthconnector_list_provider($js = NULL) {
  $header = array(
    array('data' => t('Title'),        'class' => 'oauthconnector-provider-title'),
    array('data' => t('URL'),          'class' => 'oauthconnector-provider-url'),
    array('data' => t('Endpoint'),     'class' => 'oauthconnector-provider-endpoint'),
    array('data' => t('Consumer Key'), 'class' => 'oauthconnector-provider-consumer-key'),
    array('data' => t('Storage'),      'class' => 'oauthconnector-provider-storage'),
    array('data' => t('Operations'),   'class' => 'oauthconnector-provider-operations'),
  );

  $providers = oauthconnector_provider_load_all();
  $rows = array();

  foreach ($providers as $provider) {
    $operations = array();

//    if (empty($provider->disabled)) {
      $operations[] = array(
        'title' => t('Edit'),
        'href' => 'admin/build/oauthconnector/' . $provider->name . '/edit',
      );
      $operations[] = array(
        'title' => t('Export'),
        'href' => 'admin/build/oauthconnector/' . $provider->name . '/export',
      );
//    }

    $rows[$provider->name] = array(
      'data' => array(
        'title'        => check_plain($provider->title),
        'url'          => l($provider->url, $provider->url),
        'endpoint'     => check_plain($provider->endpoint),
        'consumer_key' => check_plain($provider->consumer_key),
        'storage'      => $provider->type,
        'operations'   => theme('links', $operations),
      ),
//      'class' => (!empty($provider->disabled) ? ' oauthconnector-provider-disabled' : ''),
    );

    if (empty($rows[$provider->name]['data']['consumer_key'])) {
      $rows[$provider->name]['data']['consumer_key'] = array(
        'data'  => t('Missing'),
        'class' => 'oauthconnector-provider-warning',
      );
    }
  }

  $table = theme('table', $header, $rows, array('id' => 'oauthconnector-list-provider'));

  //drupal_add_css(drupal_get_path('module', 'oauthconnector') . '/css/oauthconnector.admin.css');

  return $table;
}

/**
 * Handle the add provider page.
 */
function oauthconnector_add_provider() {
  $provider = oauthconnector_provider_new();
  drupal_set_title(t('Add provider'));
  return oauthconnector_edit_provider($provider);
}

/**
 * Edit a provider.
 *
 * Called from both the add and edit points to provide for common flow.
 */
function oauthconnector_edit_provider($provider) {
  if (!is_object($provider)) {
    $provider = oauthconnector_provider_load($provider);
  }
  if ($provider && !empty($provider->title)) {
    drupal_set_title(check_plain($provider->title));
  }

  return drupal_get_form('oauthconnector_edit_form_provider', $provider);
}

/**
 * Form to edit the settings of a provider.
 */
function oauthconnector_edit_form_provider(&$form_state, $provider) {
  $form = array();

  $form['pid'] = array(
    '#type'  => 'value',
    '#value' => isset($provider->pid) ? $provider->pid : '',
  );
  $form['provider'] = array(
    '#type'  => 'value',
    '#value' => $provider,
  );

  $form['name'] = array(
    '#type'          => 'textfield',
    '#size'          => 24,
    '#default_value' => $provider->name,
    '#title'         => t('Name'),
    '#description'   => t('A unique machine-readable name used to identify this provider internally. It may only contain lowercase alphanumeric characters and underscores. No spaces or uppercase characters.'),
    '#required'      => TRUE,
  );

  $form['title'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Title'),
    '#description'   => t('A human-readable title for the provider.'),
    '#size'          => 32,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#default_value' => $provider->title,
  );

  $form['url'] = array(
    '#type'        => 'textfield',
    '#title'       => t('URL'),
    '#description' => t('A URL to the OAuth provider, typically to the front page.'),
    '#size'        => 32,
    '#maxlength'   => 255,
    '#required'    => TRUE,
    '#default_value' => $provider->url,
  );

  $form['endpoint'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Endpoint'),
    '#description'   => t('The API endpoint for the user info action.'),
    '#size'          => 32,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#default_value' => $provider->endpoint,
  );

  //TODO: Check that the limits for consumer key and secret are sensible - do the OAuth standard say anything about the length of these?

  $form['consumer_key'] = array(
    '#type'          => 'textfield',
    '#title'         => t('OAuth Consumer Key'),
    '#description'   => t('Your consumer key provided by the OAuth provider.'),
    '#size'          => 32,
    '#maxlength'     => 32,
    '#required'      => TRUE,
    '#default_value' => $provider->consumer_key,
  );

  $form['consumer_secret'] = array(
    '#type'          => 'textfield',
    '#title'         => t('OAuth Consumer Secret'),
    '#description'   => t('Your consumer secret provided by the OAuth provider.'),
    '#size'          => 32,
    '#maxlength'     => 32,
    '#required'      => TRUE,
    '#default_value' => $provider->consumer_secret,
  );

  $label = empty($provider->pid) ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Validate submission of the provider edit form.
 */
function oauthconnector_edit_form_provider_validate($form, &$form_state) {
  $values = $form_state['values'];

  // Test uniqueness of name
  if (preg_match("/[^a-z0-9_]/", $values['name'])) {
    form_error($form['name'], t('The name may only contain lowercase alphanumeric characters and underscores.'));
  }
  else {
    $query = "SELECT pid FROM {oauthconnector_provider} WHERE name = '%s'";
    if (!empty($values['pid']) && is_numeric($values['pid'])) {
      $query .= ' AND pid != ' . $values['pid'];
    }
    if (db_result(db_query($query, $values['name']))) {
      form_error($form['name'], t('The name must be unique.'));
    }
  }
  if (!valid_url($values['url'])) {
    form_error($form['url'], t('The url is not valid.'));
  }
  if (!valid_url($values['endpoint'])) {
    form_error($form['endpoint'], t('The endpoint is not a valid url.'));
  }
  //TODO: Maybe add some more validation?
}

/**
 * Process submission of the provider edit form.
 */
function oauthconnector_edit_form_provider_submit($form, &$form_state) {
  $provider = $form_state['values']['provider'];

  $provider->name            = $form_state['values']['name'];
  $provider->title           = $form_state['values']['title'];
  $provider->url             = $form_state['values']['url'];
  $provider->endpoint        = $form_state['values']['endpoint'];
  $provider->consumer_key    = $form_state['values']['consumer_key'];
  $provider->consumer_secret = $form_state['values']['consumer_secret'];

  if (empty($provider->pid)) {
    drupal_set_message(t('Your new provider %title has been saved.', array('%title' => $provider->title)));
    oauthconnector_provider_save($provider);
  }
  else {
    drupal_set_message(t('Your changes have been saved.'));
    oauthconnector_provider_save($provider);
  }

  $form_state['redirect'] = 'admin/build/oauthconnector';
}

/**
 * Page callback to export a provider to PHP code.
 */
function oauthconnector_export_provider(&$form_state, $provider) {
  if (!is_object($provider)) {
    $provider = oauthconnector_provider_load($provider);
  }
  drupal_set_title(check_plain($provider->title));

  $code = oauthconnector_provider_export($provider);
  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type'          => 'textarea',
    '#title'         => $provider->name,
    '#default_value' => $code,
    '#rows'          => $lines,
  );

  return $form;
}